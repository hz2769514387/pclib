#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpPoller.h" 
#include "PCTcpSockHandle.h" 
#include "PCUtilMisc_Linux.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// IO多路复用工作者线程：  为IO多路复用服务的工作者线程
//         网络请求事件到达时使用多个线程分配IO事件。
///////////////////////////////////////////////////////////////////
bool CPCTcpPollerThread::Init()
{
#if defined (_WIN32)
	m_hCompletionPort = CPCTcpPoller::GetInstance()->GetIOCPHandle();
	if (NULL == m_hCompletionPort)
	{
		PC_ERROR_LOG("CPCTcpPoller::GetInstance()->GetIOCPHandle() fail！ " );
		return false;
	}
#else
	m_epollFd = epoll_create(MAX_EPOLL_EVENTS);  
	if (m_epollFd <= 0)
	{
		PC_ERROR_LOG( "epoll_create = %d fail! errno=%d", m_epollFd, PCGetLastError());
		return false;
	}
	int nRet = pipe(m_pipeFd);
	if (nRet == -1)
	{
		m_pipeFd[0] = m_pipeFd[1] = -1;
		PC_ERROR_LOG( "pipe = -1 fail! errno=%d",  PCGetLastError());
		return false;
	}

    //将m_pipeFd[0]放入epoll队列
    nRet = LIN_EpollEventCtl(m_epollFd, m_pipeFd[0],  EPOLL_CTL_ADD, EPOLLHUP | EPOLLERR | EPOLLIN, NULL);
    if (nRet != 0)
    {
        return false;
    }
#endif
	return true;
}

void CPCTcpPollerThread::Svc()
{
#if defined (_WIN32)
    BOOL                bRet = FALSE;
    CPCTcpSockHandle*   pHandle = NULL;
    IOCP_IO_CTX*        lpIOContext = NULL;
    DWORD               dwBytesXfered = 0;

	//处理完成端口上的消息
	while (m_bRunning)
	{
		bRet = GetQueuedCompletionStatus(m_hCompletionPort, &dwBytesXfered, (PULONG_PTR)&pHandle, (LPOVERLAPPED*)&lpIOContext, INFINITE);
		if (!bRet)
		{
			//检查错误
			DWORD dwLastErr = GetLastError();
			if (dwLastErr == ERROR_INVALID_HANDLE || dwLastErr == ERROR_ABANDONED_WAIT_0)
			{
				//用户关闭了完成端口句柄，此时线程应该退出（VISTA以上系统才有效）
				PC_ERROR_LOG("user closed IOCP, CPCTcpPollerThread exit!");
				break;
			}

			if ((0 == dwBytesXfered) && (lpIOContext != NULL) && (pHandle != NULL) && (pHandle->m_SocketFd != PC_INVALID_SOCKET))
			{
                if (pHandle->m_SocketType == CPCTcpSockHandle::eSockType::eListenType)
				{
					//监听Socket取队列事件出现异常，由于监听Socket上只可能出现ACCEPT事件，此时直接关掉到来的连接
					lpIOContext->m_pOwner->ProcessClose();
				}
				else
				{
					//客户端发出的连接请求被拒绝或其他错误
					pHandle->ProcessClose();
				}
			}
			else
			{
				PC_WARN_LOG("GetQueuedCompletionStatus fail! dwLastErr = %lu, dwBytesXfered = %lu", dwLastErr, dwBytesXfered);
			}
			continue;
		}

		PC_ASSERT(pHandle, "exception!pHandle == NULL  ");
		PC_ASSERT((pHandle->m_SocketFd != PC_INVALID_SOCKET), "exception!pHandle->m_SocketFd == PC_INVALID_SOCKET");
		PC_ASSERT(lpIOContext, "exception!lpIOContext == NULL");

		//正常处理
		switch (pHandle->m_Opt)
		{
		case CPCTcpSockHandle::eOpt::eConnect:
			pHandle->ProcessConnect();
			break;
		case CPCTcpSockHandle::eOpt::eRead:
			if (0 == dwBytesXfered)
			{
				//连接被对方断开
				pHandle->ProcessClose();
			}
			else
			{
				pHandle->ProcessRecv(dwBytesXfered);
			}
			break;
		case CPCTcpSockHandle::eOpt::eWrite:
			pHandle->ProcessSend( dwBytesXfered);
			break;
		case CPCTcpSockHandle::eOpt::eAccept:
			if (dwBytesXfered > 0)
			{
				PC_WARN_LOG("cseAccept warning: %d bytes data is NOT NEED.", dwBytesXfered);
			}
			lpIOContext->m_pOwner->ProcessAccept();
			break;
		default:
			PC_ERROR_LOG("Recved unknown op type: %d", pHandle->m_Opt);
			break;
		}
	}
#else
	while (m_bRunning)
	{
        PC_ERROR_LOG("CPCTcpPollerThread epoll_waiting ... ");
        int fds = epoll_wait(m_epollFd, m_epollEvents, MAX_EPOLL_EVENTS, -1);
		if (fds < 0)
		{
            PC_ERROR_LOG("epoll_wait = %d error! errno = %d. break.", fds, PCGetLastError());
            break;
		}
		PC_ERROR_LOG("epoll_wait fds=%d, m_epollEvents[0].data.fd=%d", fds,m_epollEvents[0].data.fd );

        bool bNeedExit = false;
		for (int i = 0; i < fds; i++)
		{
			//处理eventfd事件，可能是epoll_ctl请求或退出请求
            if(m_epollEvents[i].data.ptr == NULL)
            {
				if ((m_epollEvents[i].events & EPOLLERR) || (m_epollEvents[i].events & EPOLLHUP))
				{
					PC_ERROR_LOG(" CPCTcpPollerThread process m_pipeFd[0] EPOLLERR | EPOLLHUP. continue");
					continue;
				}
				else // (m_epollEvents[i].events & EPOLLIN)
				{
					PC_TRACE_LOG(" CPCTcpPollerThread process m_pipeFd[0] EPOLLIN.");

                    int64_t eventHandlePtr = 0;
                    ssize_t readLen = read(m_pipeFd[0], &eventHandlePtr, sizeof(CPCTcpSockHandle*));
                    if(readLen!=sizeof(CPCTcpSockHandle*))
					{
                        PC_ERROR_LOG("CPCTcpPollerThread process read m_pipeFd[0] fail.readLen(%d) != sizeof(CPCTcpSockHandle*)(%d)",readLen, sizeof(CPCTcpSockHandle*));
						continue;
					}
					if(0 == eventHandlePtr)
					{
						//退出epoll_wait请求处理
						PC_TRACE_LOG("CPCTcpPollerThread process read m_pipeFd[0] data 0, need exit epoll_wait.");
						bNeedExit = true;
						continue;
					}

					/*
					//epoll_ctl请求处理
					CPCTcpSockHandle* ctrlHandle = reinterpret_cast<CPCTcpSockHandle*>(eventHandlePtr);

					struct epoll_event epv = { 0, { 0 } };
					epv.events = ctrlHandle->m_events;
					epv.data.fd = ctrlHandle->m_SocketFd;
					epv.data.ptr = ctrlHandle;
					if(0 != epoll_ctl(m_epollFd,ctrlHandle->m_epctlOp,ctrlHandle->m_SocketFd,&epv))
					{
						PC_WARN_LOG("CPCTcpPollerThread process epoll_ctl fail.socket fd = %d, op = %d, events = %d", ctrlHandle->m_SocketFd,ctrlHandle->m_epctlOp, ctrlHandle->m_events);
					}*/
				}
            }
            else
            {
                //处理实际的发送接收事件
				CPCTcpSockHandle *	eventHandle = reinterpret_cast<CPCTcpSockHandle*>(m_epollEvents[i].data.ptr);
                if (m_epollEvents[i].events & EPOLLIN)
                {
                    if(eventHandle->m_SocketType == CPCTcpSockHandle::eSockType::eListenType)
                    {
                        //到来了一个客户端
						CPCTcpSockHandle * acceptHandle  = CPCTcpPoller::GetInstance()->GetAcceptHandle();
                        if(NULL == acceptHandle)
						{
                            //没有空闲的处理连接了，直接关掉
                            sockaddr_in addrClient;
                            memset(&addrClient, 0, sizeof(sockaddr_in));
                            int addrClientlen = sizeof(addrClient);
                            int clientFd = accept(eventHandle->m_SocketFd, (sockaddr *)&addrClient, (socklen_t*)&addrClientlen);
                            PCCloseSocket(clientFd);
                        }
                        else
                        {
                            acceptHandle->ProcessAccept();
                        }
                    }
                    else
                    {
                        eventHandle->ProcessRecv(0);
                    }
                }
                if (m_epollEvents[i].events & EPOLLOUT)
                {
                    eventHandle->ProcessSend( 0);
                }
                if (m_epollEvents[i].events & EPOLLERR)
                {
                    if(eventHandle->m_SocketType == CPCTcpSockHandle::eSockType::eListenType)
                    {
                        //listen socket上产生了错误，忽略
                        PC_WARN_LOG("CPCTcpPollerThread  listen socket get  EPOLLERR fail. ignored.");
                    }
                    else
                    {
                        eventHandle->ProcessClose();
                    }
                }
            }
		}

        //等待处理完所有事情在判断是否要退出
        if(bNeedExit)
        {
            break;
        }
	}

	if (m_epollFd > 0)
	{
		close(m_epollFd);
		m_epollFd = -1;
	}
	if (m_pipeFd[0] != -1)
	{
		close(m_pipeFd[0]);
		m_pipeFd[0] = -1;
	}
	if (m_pipeFd[1] != -1)
	{
		close(m_pipeFd[1]);
		m_pipeFd[1] = -1;
	}
#endif
}



///////////////////////////////////////////////////////////////////
// IO多路复用单例类：  提供启动和停止等管理机制
///////////////////////////////////////////////////////////////////
CPCTcpPoller::CPCTcpPoller(void) 
{
#if defined (_WIN32)
	m_hCompletionPort = NULL;
#else
#endif
	m_nWorkerThreadCount = 0;
}

CPCTcpPoller::~CPCTcpPoller(void)
{
	this->StopTcpPoller();
}

bool CPCTcpPoller::StartTcpPoller()
{
	unsigned int nPollerThreadCount = 0;
#if defined (_WIN32)
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hCompletionPort)
	{
		PC_ERROR_LOG("Init CreateIoCompletionPort fail！ errno = %d" , PCGetLastError());
		return false;
	}

	//对于windows，采用cpu核心数*2的事件派发线程数
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	nPollerThreadCount = 2*si.dwNumberOfProcessors;
	if(nPollerThreadCount > MAX_POLLER_THREAD_COUNT)
	{
		PC_ERROR_LOG("nPollerThreadCount = %d > (%d) fail." , nPollerThreadCount, MAX_POLLER_THREAD_COUNT);
		return false;
	}
#else
	//对于linux，事件派发线程数1个就足够了
	nPollerThreadCount = 1;
#endif
	
	// 建立工作者线程
	m_nWorkerThreadCount = 0;
	for (unsigned int i = 0; i < nPollerThreadCount; i++)
	{
		m_phWorkerThreadList[i] = new CPCTcpPollerThread();
		m_nWorkerThreadCount++;
		if (false == m_phWorkerThreadList[i]->Init())
		{
			return false;
		}
        if (false == m_phWorkerThreadList[i]->StartThread(PC_THREAD_TIMEOUT_MS))
		{
			return false;
		}
	}

	//建立管理连接线程
    if (false == this->StartThread(PC_THREAD_TIMEOUT_MS))
	{
		return false;
	}
	PC_TRACE_LOG(" Create CPCTcpPollerThread = %d StartTcpPoller ok.", nPollerThreadCount);
	return true;
}

void CPCTcpPoller::StopTcpPoller()
{
#if defined (_WIN32)
	if (m_hCompletionPort)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
#else
#endif

	//退出线程
	for (unsigned int i = 0; i < m_nWorkerThreadCount; i++)
	{
#if defined (_WIN32)
#else
		//发送停止epoll_wait的请求
		if (m_phWorkerThreadList[i])
		{
            int64_t exitCode = 0;
            m_phWorkerThreadList[i]->SendThreadMsg(&exitCode, sizeof(CPCTcpSockHandle*));
		}
#endif
		if (m_phWorkerThreadList[i])
		{
			delete m_phWorkerThreadList[i];
			m_phWorkerThreadList[i] = NULL;
		}
	}
    m_nWorkerThreadCount = 0;

	PC_TRACE_LOG(" StopTcpPoller all ok.");
}

void CPCTcpPoller::Svc()
{
	while (m_bRunning)
	{
		PCSleepMsec(800);
	}
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
