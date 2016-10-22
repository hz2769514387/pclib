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
void CPCTcpPollerThread::Svc()
{
#if defined (_WIN32)
	HANDLE			hCompletionPort = CPCTcpPoller::GetInstance()->GetIOCPHandle();

	BOOL			bRet = FALSE;
	CPCTcpSockHandle *pHandle = NULL;
	IOCP_IO_CTX    *lpIOContext = NULL;
	DWORD           dwBytesXfered = 0;

	//������ɶ˿��ϵ���Ϣ
	while (m_bRunning)
	{
		bRet = GetQueuedCompletionStatus(hCompletionPort,&dwBytesXfered,(PULONG_PTR)&pHandle,(LPOVERLAPPED*)&lpIOContext,PER_GET_POLLER_QUEUE_WAIT_TIME);
		if (!bRet)
		{
			//������
			DWORD dwLastErr = GetLastError();
			if (dwLastErr == WAIT_TIMEOUT)
			{
				continue;
			}
			else if (dwLastErr == ERROR_INVALID_HANDLE)
			{
				//�û��ر�����ɶ˿ھ������ʱ�߳�Ӧ���˳���VISTA����ϵͳ����Ч��
				PC_ERROR_LOG("user closed IOCP, CPCTcpPollerThread exit!");
				break;
			}

			if ((0 == dwBytesXfered) && (lpIOContext != NULL) && (pHandle != NULL) && (pHandle->m_hTcpSocket != PC_INVALID_SOCKET))
			{
				if (pHandle->m_bListenSocket)
				{
					//����Socketȡ�����¼������쳣�����ڼ���Socket��ֻ���ܳ���ACCEPT�¼�������ֱ�ӹص�����������
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

		//��������
		switch (lpIOContext->m_byOpType)
		{
		case OP_CONNECT:
			pHandle->DoConnected(true);
			break;
		case OP_READ:
			if (0 == dwBytesXfered)
			{
				//���ӱ��Է��Ͽ�
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
			PC_ERROR_LOG("�յ�δ֪��Ϣ %02x", lpIOContext->m_byOpType);
			break;
		}
		delete lpIOContext;
	}

#else

#endif
}



///////////////////////////////////////////////////////////////////
// IO��·���õ����ࣺ  �ṩ������ֹͣ�ȹ������
///////////////////////////////////////////////////////////////////
CPCTcpPoller::CPCTcpPoller(void) :
	m_hCompletionPort(NULL)
{
}

CPCTcpPoller::~CPCTcpPoller(void)
{
	this->StopTcpPoller();
}

bool CPCTcpPoller::StartTcpPoller(unsigned int nPollerThreadCount)
{
#if defined (_WIN32)
	//�Ѿ���ʼ������
	if (m_hCompletionPort)
	{
		return true;
	}

	// ������һ����ɶ˿�
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == m_hCompletionPort)
	{
		PC_ERROR_LOG("Init CreateIoCompletionPort fail�� errno = %d" , PCGetLastError());
		return false;
	}
#endif
	
	// �����������߳�
	for (unsigned int i = 0; i < nPollerThreadCount; i++)
	{
		CPCTcpPollerThread* phWorkerThread = new CPCTcpPollerThread();
		m_phWorkerThreadList.push_back(phWorkerThread);
		if (false == phWorkerThread->StartThread(5000))
		{
			return false;
		}
	}

	PC_TRACE_LOG(" Create CPCTcpPollerThread = %d StartTcpPoller ok.", nPollerThreadCount);
	return true;
}

void CPCTcpPoller::StopTcpPoller()
{
	//�˳��߳�
	for (auto it = m_phWorkerThreadList.begin(); it != m_phWorkerThreadList.end(); it++)
	{
		CPCTcpPollerThread* phWorkerThread = *it;
		phWorkerThread->StopThread(500);
		delete phWorkerThread;
	}
	m_phWorkerThreadList.clear();

#if defined (_WIN32)
	// �ر�IOCP���
	if (m_hCompletionPort)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
#endif

	PC_TRACE_LOG(" StopTcpPoller all ok.");
}

bool CPCTcpPoller::AssociateSocketWithIOCP(PC_SOCKET socket, ULONG_PTR dwCompletionKey)
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

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
