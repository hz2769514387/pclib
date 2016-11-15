#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpPoller.h" 
#include "PCTcpSockHandle.h" 

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

    //��m_pipeFd[0]����epoll����
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

			if ((0 == dwBytesXfered) && (lpIOContext != NULL) && (pHandle != NULL) && (pHandle->m_SocketFd != PC_INVALID_SOCKET))
			{
                if (pHandle->m_SocketType == CPCTcpSockHandle::eSockType::eListenType)
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
		PC_ASSERT((pHandle->m_SocketFd != PC_INVALID_SOCKET), "exception!pHandle->m_SocketFd == PC_INVALID_SOCKET");
		PC_ASSERT(lpIOContext, "exception!lpIOContext == NULL");

		//��������
		switch (pHandle->m_Opt)
		{
		case CPCTcpSockHandle::eOpt::eConnect:
			pHandle->ProcessConnect();
			break;
		case CPCTcpSockHandle::eOpt::eRead:
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

                //����eventfd�¼���������epoll_ctl������˳�����
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
                //����ʵ�ʵķ��ͽ����¼�
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

        //�ȴ������������������ж��Ƿ�Ҫ�˳�
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
// IO��·���õ����ࣺ  �ṩ������ֹͣ�ȹ������
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
#if defined (_WIN32)
#else
		m_pPollerThread = m_phWorkerThreadList[i];
#endif
	}

	//�������������߳�
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
    //����ֹͣepoll_wait������
    int64_t exitCode = 0;
	ssize_t exitByteLen = write(m_pPollerThread->m_pipeFd[1], &exitCode, sizeof(int64_t));
    if(exitByteLen != sizeof(int64_t))
    {
        PC_WARN_LOG(" write exit code fail! exitByteLen = %d", exitByteLen);
    }
#endif

	//�˳��߳�
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
