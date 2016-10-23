#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpSockHandle.h"
#include "PCTcpPoller.h" 

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
		PC_ERROR_LOG("CPCTcpPoller::GetInstance()->GetIOCPHandle()  == NULL ");
		return false;
	}
#else
	m_epollFd = epoll_create(MAX_EPOLL_EVENTS);  
	if (m_epollFd <= 0)
	{
		CLOG_ERROR(pLog, "epoll_create = %d fail! errno=%d", m_epollFd, PCGetLastError());
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
		bRet = GetQueuedCompletionStatus(m_hCompletionPort, &dwBytesXfered, (PULONG_PTR)&pHandle, (LPOVERLAPPED*)&lpIOContext, PER_GET_POLLER_QUEUE_WAIT_TIME);
		if (!bRet)
		{
			//检查错误
			DWORD dwLastErr = GetLastError();
			if (dwLastErr == WAIT_TIMEOUT)
			{
				continue;
			}
			else if (dwLastErr == ERROR_INVALID_HANDLE)
			{
				//用户关闭了完成端口句柄，此时线程应该退出（VISTA以上系统才有效）
				PC_ERROR_LOG("user closed IOCP, CPCTcpPollerThread exit!");
				break;
			}

			if ((0 == dwBytesXfered) && (lpIOContext != NULL) && (pHandle != NULL) && (pHandle->m_hTcpSocket != PC_INVALID_SOCKET))
			{
				if (pHandle->m_bListenSocket)
				{
					//监听Socket取队列事件出现异常，由于监听Socket上只可能出现ACCEPT事件，此事直接关掉到来的连接
					lpIOContext->m_pOwner->DoClose();
				}
				else
				{
					pHandle->DoClose();
				}
			}
			delete lpIOContext;
			continue;
		}

		PC_ASSERT(pHandle, "exception!pHandle == NULL  ");
		PC_ASSERT((pHandle->m_hTcpSocket != PC_INVALID_SOCKET), "exception!pHandle->m_hTcpSocket == PC_INVALID_SOCKET");
		PC_ASSERT(lpIOContext, "exception!lpIOContext == NULL");

		//正常处理
		switch (lpIOContext->m_byOpType)
		{
		case OP_CONNECT:
			pHandle->DoConnected(true);
			break;
		case OP_READ:
			if (0 == dwBytesXfered)
			{
				//连接被对方断开
				pHandle->DoClose();
			}
			else
			{
				pHandle->DoRecved(true, lpIOContext->m_szIOBuf, dwBytesXfered);
			}
			break;
		case OP_WRITE:
			pHandle->DoSendded(true, dwBytesXfered);
			break;
		case OP_ACCEPT:
			lpIOContext->m_pOwner->DoAccept(true, lpIOContext->m_szIOBuf, dwBytesXfered);
			break;
		default:
			PC_ERROR_LOG("Recved unknown op type: %d", lpIOContext->m_byOpType);
			break;
		}
		delete lpIOContext;
	}
#else
	while (m_bRunning)
	{
		int fds = epoll_wait(m_epollFd, m_epollEvents, MAX_EPOLL_EVENTS, PER_GET_POLLER_QUEUE_WAIT_TIME);
		if (fds < 0)
		{
			PC_ERROR_LOG("epoll_wait = %d error! errno = %d. continue.", fds, PCGetLastError());
			PCSleepMsec(PER_GET_POLLER_QUEUE_WAIT_TIME);
			continue;
		}

		for (int i = 0; i < fds; i++)
		{
			CPCTcpSockHandle *	eventHandle = (CPCTcpSockHandle *)m_epollEvents[i].data.ptr;
			if(eventHandle == NULL)
			{
				PC_ERROR_LOG("eventHandle = NULL, continue.");
				continue;
			}
			if (m_epollEvents[i].events & EPOLLIN)
			{
				if(eventHandle->m_bListenSocket)
				{
					eventHandle->OnAccept();
				}
				else
				{
					eventHandle->OnReceive(0);
				}
			}
			if (m_epollEvents[i].events & EPOLLOUT)
			{
				eventHandle->OnSend(0);
			}
			if (m_epollEvents[i].events & EPOLLERR)
			{
				eventHandle->OnClose();
			}
		}
	}
	close(m_epollFd);
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
	m_dwCurrentEpollFd = 0;
#endif
	m_nWorkerThreadCount = 0;
}

CPCTcpPoller::~CPCTcpPoller(void)
{
	this->StopTcpPoller();
}

bool CPCTcpPoller::StartTcpPoller(unsigned int nPollerThreadCount)
{
#if defined (_WIN32)
	//已经初始化过了
	if (m_hCompletionPort)
	{
		return true;
	}

	// 建立第一个完成端口
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hCompletionPort)
	{
		PC_ERROR_LOG("Init CreateIoCompletionPort fail！ errno = %d" , PCGetLastError());
		return false;
	}
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
	}

	PC_TRACE_LOG(" Create CPCTcpPollerThread = %d StartTcpPoller ok.", nPollerThreadCount);
	return true;
}

void CPCTcpPoller::StopTcpPoller()
{
	//退出线程
	for (unsigned int i = 0; i < m_nWorkerThreadCount; i++)
	{
		m_phWorkerThreadList[i]->StopThread(500);
		delete m_phWorkerThreadList[i];
	}

#if defined (_WIN32)
	// 关闭IOCP句柄
	if (m_hCompletionPort)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
#endif

	PC_TRACE_LOG(" StopTcpPoller all ok.");
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
