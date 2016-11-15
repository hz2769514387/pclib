#ifndef _PC_TCPPOLLER_H_
#define _PC_TCPPOLLER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCThread.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//���IO��·���ù����̸߳���
#define MAX_POLLER_THREAD_COUNT	(128)


/**
*@brief		�ض�ƽ̨�첽ģ�͵Ľṹ����
*/
class CPCTcpSockHandle;
#if defined (_WIN32)

	//ÿ����IOCPͶ����������ݽṹ��������Ҫ��Ϊ�˱���AcceptExʱ�Ĵ���ָ��
	typedef struct _IOCP_IO_CTX
	{
		OVERLAPPED		m_olOriginal;	//ԭʼ�ص��ṹ
		CPCTcpSockHandle*	m_pOwner;	//���Ͷ�ݵ����������ڵ�����
	} IOCP_IO_CTX;

#else

	//epollһ�εȴ����¼�����
	#define MAX_EPOLL_EVENTS        (100)
#endif

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
    int     m_epollFd;
    int     m_pipeFd[2];
	struct	epoll_event m_epollEvents[MAX_EPOLL_EVENTS];

	bool	SendThreadMsg(void *Msg , size_t nMsgLen)
	{
		ssize_t sLen = write(m_pipeFd[1], Msg, nMsgLen);
		if (sLen != nMsgLen)
		{
			PC_ERROR_LOG(" write exit code fail! exitByteLen = %d", exitByteLen);
		}
	}
#endif
};

/**
*@brief	IO��·���õ�����
*
*/
class CPCTcpPoller  : public CPCThread
{
public:
	//��ȡ����
	static CPCTcpPoller* GetInstance(){static CPCTcpPoller m_me;return &m_me;}

	//��CPCTcpSockHandle�󶨻���CPCTcpPoller
	void BindTcpSockHandle(CPCTcpSockHandle* hTcpSockHandle)
	{
		CPCGuard guard(m_Mutex);
		if (hTcpSockHandle)
		{
			m_TcpSockHandleList.insert(hTcpSockHandle);
		}
	}
	void UnBindTcpSockHandle(CPCTcpSockHandle* hTcpSockHandle)
	{
		CPCGuard guard(m_Mutex);
		if (hTcpSockHandle)
		{
			m_TcpSockHandleList.erase(hTcpSockHandle);
		}
	}

	//������ֹͣ
	bool StartTcpPoller();
	void StopTcpPoller();

	//����
	void Svc();

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
    //��ȡCPCTcpPollerThreadָ��
	CPCTcpPollerThread* GetPollerThread(){ return m_phWorkerThreadList[i]; }
#endif
	
protected:
	CPCTcpPoller(void);
	~CPCTcpPoller(void);

	CPCRecursiveLock	m_Mutex;

	// �����߳��б�
	CPCTcpPollerThread* m_phWorkerThreadList[MAX_POLLER_THREAD_COUNT];
	unsigned int		m_nWorkerThreadCount;

	// ���������б�
	std::set<CPCTcpSockHandle*>	m_TcpSockHandleList;

#if defined (_WIN32)
	// ��ɶ˿ڵľ��
	HANDLE				m_hCompletionPort;
#else
    // CPCTcpPollerThreadָ��
	CPCTcpPollerThread*	m_pPollerThread;
#endif
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPPOLLER_H_*/
