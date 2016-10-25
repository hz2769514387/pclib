#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//ÿ��ȡһ��ʱ��������ݵĵȴ���ʱʱ��(����)
#define PER_GET_POLLER_QUEUE_WAIT_TIME	(50)

//���IO��·���ù����̸߳���
#define MAX_POLLER_THREAD_COUNT	(64)

/**
*@brief	IO��·���ù������߳�
*
*/
class CPCTcpPollerThread : public CPCThread
{
public:
	bool Init();
	void Svc();

public:

#if defined (_WIN32)
	//��ɶ˿ڵľ��
	HANDLE	m_hCompletionPort;
#else
	//epoll���������
	PC_SOCKET	m_epollFd;
	struct	epoll_event m_epollEvents[MAX_EPOLL_EVENTS];
#endif
};

/**
*@brief	IO��·���õ�����
*
*/
class CPCTcpPoller: CPCNoCopyable
{
public:
	//��ȡ����
	static CPCTcpPoller* GetInstance(){static CPCTcpPoller m_me;return &m_me;}

	//������ֹͣ
	bool StartTcpPoller(unsigned int nPollerThreadCount = 10);
	void StopTcpPoller();

#if defined (_WIN32)
	//��SOCKET����󶨵���ɶ˿��� 
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
	//��ȡ��ɶ˿ھ��
	HANDLE	GetIOCPHandle(){ return m_hCompletionPort; }
#else
	//������ÿ��CPCTcpPollerThread��ȡEpoll���
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

	// �����߳��б�
	CPCTcpPollerThread* m_phWorkerThreadList[MAX_POLLER_THREAD_COUNT];
	unsigned int	m_nWorkerThreadCount;

	CPCRecursiveLock	m_Mutex;			

#if defined (_WIN32)
	// ��ɶ˿ڵľ��
	HANDLE			m_hCompletionPort;
#else
	// �ֵ���epoll���
	unsigned int	m_dwCurrentEpollFd;
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
