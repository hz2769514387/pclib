#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpSockHandle.h"
#include "PCTcpPoller.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// IO��·���ù������̣߳�  ΪIO��·���÷���Ĺ������߳�
//         ���������¼�����ʱʹ�ö���̷߳���IO�¼���
///////////////////////////////////////////////////////////////////
bool CPCTcpPollerThread::Init()
{
#if defined (_WIN32)
	m_hCompletionPort = CPCTcpPoller::GetInstance()->GetIOCPHandle();
#else
	m_epollFd = CPCTcpPoller::GetInstance()->GetEpollFd();
    m_eventFd = CPCTcpPoller::GetInstance()->GetEventFd();

    //��eventfd����epoll����
    struct epoll_event read_event;
    read_event.events = EPOLLHUP | EPOLLERR | EPOLLIN;
    read_event.data.fd = m_eventFd;
    int nRet = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_eventFd, &read_event);
    if (nRet == -1)
    {
        PC_ERROR_LOG("epoll_ctl(eventfd) = %d, errno = %d fail! ", nRet, PCGetLastError());
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

	//������ɶ˿��ϵ���Ϣ
	while (m_bRunning)
	{
		bRet = GetQueuedCompletionStatus(m_hCompletionPort, &dwBytesXfered, (PULONG_PTR)&pHandle, (LPOVERLAPPED*)&lpIOContext, INFINITE);
		if (!bRet)
		{
			//������
			DWORD dwLastErr = GetLastError();
			if (dwLastErr == ERROR_INVALID_HANDLE || dwLastErr == ERROR_ABANDONED_WAIT_0)
			{
				//�û��ر�����ɶ˿ھ������ʱ�߳�Ӧ���˳���VISTA����ϵͳ����Ч��
				PC_ERROR_LOG("user closed IOCP, CPCTcpPollerThread exit!");
				break;
			}

			if ((0 == dwBytesXfered) && (lpIOContext != NULL) && (pHandle != NULL) && (pHandle->m_hTcpSocket != PC_INVALID_SOCKET))
			{
				if (pHandle->m_bListenSocket)
				{
					//����Socketȡ�����¼������쳣�����ڼ���Socket��ֻ���ܳ���ACCEPT�¼�����ʱֱ�ӹص�����������
					lpIOContext->m_pOwner->ProcessClose();
				}
				else
				{
					//�ͻ��˷������������󱻾ܾ�����������
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
		PC_ASSERT((pHandle->m_hTcpSocket != PC_INVALID_SOCKET), "exception!pHandle->m_hTcpSocket == PC_INVALID_SOCKET");
		PC_ASSERT(lpIOContext, "exception!lpIOContext == NULL");

		//��������
		switch (pHandle->m_ConnOpt)
		{
		case CPCTcpSockHandle::csEnumOpt::cseConnect:
			pHandle->ProcessConnect();
			break;
		case CPCTcpSockHandle::csEnumOpt::cseRead:
			if (0 == dwBytesXfered)
			{
				//���ӱ��Է��Ͽ�
				pHandle->ProcessClose();
			}
			else
			{
				pHandle->ProcessRecv(dwBytesXfered);
			}
			break;
		case CPCTcpSockHandle::csEnumOpt::cseWrite:
			pHandle->ProcessSend( dwBytesXfered);
			break;
		case CPCTcpSockHandle::csEnumOpt::cseAccept:
			if (dwBytesXfered > 0)
			{
				PC_WARN_LOG("cseAccept warning: %d bytes data is NOT NEED.", dwBytesXfered);
			}
			lpIOContext->m_pOwner->ProcessAccept();
			break;
		default:
			PC_ERROR_LOG("Recved unknown op type: %d", pHandle->m_ConnOpt);
			break;
		}
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
                    eventHandle->DoAccept(true, "", 0);
				}
				else
				{
                    eventHandle->DoRecved(true, "", 0);
				}
			}
			if (m_epollEvents[i].events & EPOLLOUT)
			{
                eventHandle->DoSendded(true, 0);
			}
			if (m_epollEvents[i].events & EPOLLERR)
			{
                eventHandle->DoClose();
			}
		}
	}
#endif
}



///////////////////////////////////////////////////////////////////
// IO��·���õ����ࣺ  �ṩ������ֹͣ�ȹ������
///////////////////////////////////////////////////////////////////
CPCTcpPoller::CPCTcpPoller(void) 
{
#if defined (_WIN32)
	m_hCompletionPort = NULL;
#else
	m_epollFd = -1;
    m_eventFd = -1;
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
		PC_ERROR_LOG("Init CreateIoCompletionPort fail�� errno = %d" , PCGetLastError());
		return false;
	}

	//����windows������cpu������*2���¼��ɷ��߳���
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	nPollerThreadCount = 2*si.dwNumberOfProcessors;
	if(nPollerThreadCount > MAX_POLLER_THREAD_COUNT)
	{
		PC_ERROR_LOG("nPollerThreadCount = %d > (%d) fail." , nPollerThreadCount, MAX_POLLER_THREAD_COUNT);
		return false;
	}
#else
	m_epollFd = epoll_create(MAX_EPOLL_EVENTS);  
	if (m_epollFd <= 0)
	{
        PC_ERROR_LOG( "epoll_create = %d fail! errno=%d", m_epollFd, PCGetLastError());
		return false;
	}
    m_eventFd = eventfd(0, 0);
    if (m_eventFd == -1)
    {
        PC_ERROR_LOG( "eventfd = -1 fail! errno=%d",  PCGetLastError());
        return false;
    }

	//����linux���¼��ɷ��߳���1�����㹻��
	nPollerThreadCount = 1;
#endif
	
	// �����������߳�
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
#if defined (_WIN32)
	if (m_hCompletionPort)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
#else
	if (m_epollFd > 0)
	{
		close(m_epollFd);
		m_epollFd = -1;
	}
	if (m_eventFd != -1)
	{
		close(m_eventFd);
		m_eventFd = -1;
	}
#endif

	//�˳��߳�
	for (unsigned int i = 0; i < m_nWorkerThreadCount; i++)
	{
		m_phWorkerThreadList[i]->StopThread(5000);
		delete m_phWorkerThreadList[i];
	}
	PC_TRACE_LOG(" StopTcpPoller all ok.");
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
