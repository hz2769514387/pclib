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


/**
*@brief		特定平台异步模型的结构定义
*/
class CPCTcpSockHandle;
#if defined (_WIN32)

	//每次向IOCP投递请求的数据结构，这里主要是为了保存AcceptEx时的处理指针
	typedef struct _IOCP_IO_CTX
	{
		OVERLAPPED		m_olOriginal;	//原始重叠结构
		CPCTcpSockHandle*	m_pOwner;	//这个投递的数据所属于的连接
	} IOCP_IO_CTX;

#else

	//epoll一次等待的事件数量
	#define MAX_EPOLL_EVENTS        (100)
#endif

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
    int     m_pipeFd[2];
	struct	epoll_event m_epollEvents[MAX_EPOLL_EVENTS];

	bool	SendThreadMsg(void *Msg , size_t nMsgLen)
	{
		ssize_t sLen = write(m_pipeFd[1], Msg, nMsgLen);
		if (sLen != nMsgLen)
		{
			PC_ERROR_LOG(" write Msg fail! errno = %d, nMsgLen = %d, sLen = %d",  PCGetLastError(), nMsgLen, sLen);
			return false;
		}
		return true;
	}
#endif
};

/**
*@brief	IO多路复用单例类
*
*/
class CPCTcpPoller  : public CPCThread
{
public:
	//获取单例
	static CPCTcpPoller* GetInstance(){static CPCTcpPoller m_me;return &m_me;}

	//启动和停止
	bool StartTcpPoller();
	void StopTcpPoller();

	//运行
	void Svc();

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
    //获取CPCTcpPollerThread指针
	CPCTcpPollerThread* GetPollerThread(){ return m_phWorkerThreadList[0]; }

	//绑定到等待连接列表
	void BindAcceptHandle(CPCTcpSockHandle* hTcpSockHandle)
	{
		CPCGuard guard(m_Mutex);
		if (hTcpSockHandle)
		{
			bool bRepeatHandle = false;
			for(auto it = m_WaitAcceptList.begin(); it != m_WaitAcceptList.end(); it++)
			{
				if(*it == hTcpSockHandle)
				{
					PC_WARN_LOG("hTcpSockHandle is at m_WaitAcceptList already.ignored.");
					bRepeatHandle = true;
					break;
				}
			}
			if(!bRepeatHandle)
			{
				m_WaitAcceptList.push_back(hTcpSockHandle);
			}
		}
	}

	//从等待连接列表中获取一个连接，失败则返回NULL
	CPCTcpSockHandle* GetAcceptHandle()
	{
		CPCGuard guard(m_Mutex);
		if(m_WaitAcceptList.empty())
		{	
			PC_ERROR_LOG("m_WaitAcceptSet is empty, CAN'T accept connection.");
			return NULL;
		}
		CPCTcpSockHandle* idleHandle = m_WaitAcceptList.front();
		m_WaitAcceptList.pop_front();
		return idleHandle;
	}
#endif
	
protected:
	CPCTcpPoller(void);
	~CPCTcpPoller(void);

	CPCRecursiveLock	m_Mutex;

	// 工作线程列表
	CPCTcpPollerThread* m_phWorkerThreadList[MAX_POLLER_THREAD_COUNT];
    unsigned int		m_nWorkerThreadCount;

#if defined (_WIN32)
	// 完成端口的句柄
	HANDLE				m_hCompletionPort;
#else
	// 等待接收的连接列表
	std::list<CPCTcpSockHandle*>	m_WaitAcceptList;
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
