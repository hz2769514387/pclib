#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpPoller.h" 
#include "PCTcpSockHandle.h" 

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
#else
	m_epollFd = epoll_create(MAX_EPOLL_EVENTS);  
	if (m_epollFd <= 0)
	{
		PC_ERROR_LOG( "epoll_create = %d fail! errno=%d", m_epollFd, PCGetLastError());
		return false;
	}
	m_pipeFd[0] = -1;
	m_pipeFd[1] = -1;
	int nRet = pipe(m_pipeFd);
	if (nRet == -1)
	{
		PC_ERROR_LOG( "pipe = -1 fail! errno=%d",  PCGetLastError());
		return false;
	}

    //将m_pipeFd[0]放入epoll队列
    struct epoll_event read_event;
	read_event.events = EPOLLHUP | EPOLLERR | EPOLLIN  ;
	read_event.data.fd = m_pipeFd[0];
	nRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_pipeFd[0], &read_event);
    if (nRet == -1)
    {
        PC_ERROR_LOG("epoll_ctl(m_pipeFd[0]) = %d, errno = %d fail! ", nRet, PCGetLastError());
        return false;
    }
#endif
	return true;
}

void CPCTcpPollerThread::Svc()
{
#if defined (_WIN32)
	BOOL			bRet = FALSE;
	CPCTcpSockHandle *pHandle = NULL;
	IOCP_IO_CTX    *lpIOContext = NULL;
	DWORD           dwBytesXfered = 0;

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
        int fds = epoll_wait(m_epollFd, m_epollEvents, MAX_EPOLL_EVENTS, -1);
		if (fds < 0)
		{
            PC_ERROR_LOG("epoll_wait = %d error! errno = %d. continue.", fds, PCGetLastError());
            break;
		}
        bool bNeedExit = false;
		for (int i = 0; i < fds; i++)
		{
			if(m_epollEvents[i].data.fd == m_pipeFd[0])
            {
				PC_TRACE_LOG(" CPCTcpPollerThread process eventfd event.");
				if (m_epollEvents[i].events & EPOLLIN)
				{
					PC_TRACE_LOG(" CPCTcpPollerThread process eventfd EPOLLIN.");
				}
				if (m_epollEvents[i].events & EPOLLOUT)
				{
					PC_TRACE_LOG(" CPCTcpPollerThread process eventfd EPOLLOUT.");
				}
				if (m_epollEvents[i].events & EPOLLERR)
				{
					PC_TRACE_LOG(" CPCTcpPollerThread process eventfd EPOLLERR.");
				}
				if (m_epollEvents[i].events & EPOLLHUP)
				{
					PC_TRACE_LOG(" CPCTcpPollerThread process eventfd EPOLLHUP.");
				}

                //处理eventfd事件，可能是epoll_ctl请求或退出请求
                CPCTcpSockHandle *	eventHandle = NULL;
				ssize_t readLen = read(m_pipeFd[0], &eventHandle, sizeof(eventHandle));
				PC_TRACE_LOG(" CPCTcpPollerThread read(%d) bytes.",readLen );

                if(readLen!=sizeof(eventHandle))
                {
                    PC_ERROR_LOG("read m_eventFd fail.readLen=%d",readLen);
                    continue;
                }
                if(0 == eventHandle)
                {
                    PC_TRACE_LOG("read m_eventFd data 0, need exit epoll_wait.");
                    bNeedExit = true;
                    continue;
                }
            }
            else
            {
				PC_TRACE_LOG(" CPCTcpPollerThread process network event.");
                //处理实际的发送接收事件
                CPCTcpSockHandle *	eventHandle = (CPCTcpSockHandle *)m_epollEvents[i].data.ptr;
                if(eventHandle == NULL)
                {
                    PC_ERROR_LOG("eventHandle = NULL, continue.");
                    continue;
                }
                if (m_epollEvents[i].events & EPOLLIN)
                {
                    if(eventHandle->m_SocketType == CPCTcpSockHandle::eSockType::eListenType)
                    {
                        eventHandle->ProcessAccept();
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
                    eventHandle->ProcessClose();
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
	m_pPollerThread = NULL;
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
		if (false == m_phWorkerThreadList[i]->StartThread(5000))
		{
			return false;
		}
#if defined (_WIN32)
#else
		m_pPollerThread = m_phWorkerThreadList[i];
#endif
	}

	//建立管理连接线程
	if (false == this->StartThread(5000))
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
    //发送停止epoll_wait的请求
    int64_t exitCode = 0;
	ssize_t exitByteLen = write(m_pPollerThread->m_pipeFd[1], &exitCode, sizeof(int64_t));
    if(exitByteLen != sizeof(int64_t))
    {
        PC_WARN_LOG(" write exit code fail! exitByteLen = %d", exitByteLen);
    }
#endif

	//退出线程
	for (unsigned int i = 0; i < m_nWorkerThreadCount; i++)
	{
		m_phWorkerThreadList[i]->StopThread(5000);
		delete m_phWorkerThreadList[i];
	}

	this->StopThread(5000);
	PC_TRACE_LOG(" StopTcpPoller all ok.");
}

void CPCTcpPoller::Svc()
{
	while (m_bRunning)
	{
		PC_WARN_LOG(" CPCTcpPoller .. ");
		PCSleepMsec(800);
	}
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
