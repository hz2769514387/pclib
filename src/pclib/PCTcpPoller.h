#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//最大IO多路复用工作线程个数
#define MAX_POLLER_THREAD_COUNT	(128)

//epoll一次等待的事件数量
#define MAX_EPOLL_EVENTS        (100)

/**
*@brief	IO多路复用工作者线程
*
*/
class CPCTcpPollerThread : public CPCThread
{
public:
	bool Init();
	void Svc();

public:

#if defined (_WIN32)
	//完成端口的句柄
	HANDLE	m_hCompletionPort;
#else
	//epoll句柄和数据
    int     m_epollFd;
    int     m_eventFd;
	struct	epoll_event m_epollEvents[MAX_EPOLL_EVENTS];
#endif
};

/**
*@brief	IO多路复用单例类
*
*/
class CPCTcpPoller: CPCNoCopyable
{
public:
	//获取单例
	static CPCTcpPoller* GetInstance(){static CPCTcpPoller m_me;return &m_me;}

	//启动和停止
	bool StartTcpPoller();
	void StopTcpPoller();

#if defined (_WIN32)
	//将SOCKET句柄绑定到完成端口中 
	bool	AssociateSocketWithIOCP(PC_SOCKET socket, ULONG_PTR dwCompletionKey)
	{
		if (m_hCompletionPort == NULL)
		{
			PC_ERROR_LOG("AssociateSocketWithIOCP fail. m_hCompletionPort == NULL");
			return false;
		}
		HANDLE hCompletionPort = CreateIoCompletionPort((HANDLE)socket, m_hCompletionPort, dwCompletionKey, 0);
		if (hCompletionPort != m_hCompletionPort)
		{
			PC_ERROR_LOG("CreateIoCompletionPort fail. hCompletionPort != m_hCompletionPort");
			return false;
		}
		return true;
	}
	//获取完成端口句柄
	HANDLE	GetIOCPHandle(){ return m_hCompletionPort; }
#else
    //获取Epoll句柄和事件通知句柄
    int GetEpollFd(){ return m_epollFd; }
    int GetEventFd(){ return m_eventFd; }
#endif
	
protected:
	CPCTcpPoller(void);
	~CPCTcpPoller(void);

	// 工作线程列表
	CPCTcpPollerThread* m_phWorkerThreadList[MAX_POLLER_THREAD_COUNT];
	unsigned int	m_nWorkerThreadCount;

	CPCRecursiveLock	m_Mutex;			

#if defined (_WIN32)
	// 完成端口的句柄
	HANDLE		m_hCompletionPort;
#else
    // epoll句柄和事件通知句柄
    int     	m_epollFd;
    int         m_eventFd;
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
