#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//每获取一次时间队列数据的等待超时时间(毫秒)
#define PER_GET_POLLER_QUEUE_WAIT_TIME	(50)

//最大IO多路复用工作线程个数
#define MAX_POLLER_THREAD_COUNT	(64)

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
	PC_SOCKET	m_epollFd;
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
	bool StartTcpPoller(unsigned int nPollerThreadCount = 10);
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
	//轮流从每个CPCTcpPollerThread获取Epoll句柄
	PC_SOCKET GetEpollFd()
	{
		CPCGuard guard(m_Mutex);
		if (m_dwCurrentEpoll >= m_nWorkerThreadCount)
		{
			m_dwCurrentEpoll = 0;
		}
		PC_TRACE_LOG( "Get NO.%d epoll fd.",m_dwCurrentEpoll);
		return m_phWorkerThreadList[m_dwCurrentEpoll++]->m_epollFd;
	}
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
	HANDLE			m_hCompletionPort;
#else
	// 轮到的epoll句柄
	unsigned int	m_dwCurrentEpollFd;
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
