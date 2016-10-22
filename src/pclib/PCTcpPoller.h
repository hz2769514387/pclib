#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//ÿ��ȡһ��ʱ��������ݵĵȴ���ʱʱ��
#define PER_GET_POLLER_QUEUE_WAIT_TIME	(50)

/**
*@brief	IO��·���ù������߳�
*
*/
class CPCTcpPollerThread : public CPCThread
{
public:
	void Svc();
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
	bool	AssociateSocketWithIOCP(PC_SOCKET socket, ULONG_PTR dwCompletionKey);
	//��ȡ��ɶ˿ھ��
	HANDLE	GetIOCPHandle(){ return m_hCompletionPort; }
#else
	//������ÿ��CPCTcpPollerThread��ȡEpoll���
	PC_SOCKET GetEpollFd();
#endif
	
protected:
	CPCTcpPoller(void);
	~CPCTcpPoller(void);

	// �����߳��б�
	std::list<CPCTcpPollerThread*> m_phWorkerThreadList;
	
#if defined (_WIN32)
	// ��ɶ˿ڵľ��
	HANDLE	m_hCompletionPort;
#else
	
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
