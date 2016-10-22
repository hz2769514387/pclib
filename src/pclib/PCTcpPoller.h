#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//每获取一次时间队列数据的等待超时时间
#define PER_GET_POLLER_QUEUE_WAIT_TIME	(50)

/**
*@brief	IO多路复用工作者线程
*
*/
class CPCTcpPollerThread : public CPCThread
{
public:
	void Svc();
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
	bool	AssociateSocketWithIOCP(PC_SOCKET socket, ULONG_PTR dwCompletionKey);
	//获取完成端口句柄
	HANDLE	GetIOCPHandle(){ return m_hCompletionPort; }
#else
	//轮流从每个CPCTcpPollerThread获取Epoll句柄
	PC_SOCKET GetEpollFd();
#endif
	
protected:
	CPCTcpPoller(void);
	~CPCTcpPoller(void);

	// 工作线程列表
	std::list<CPCTcpPollerThread*> m_phWorkerThreadList;
	
#if defined (_WIN32)
	// 完成端口的句柄
	HANDLE	m_hCompletionPort;
#else
	
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
