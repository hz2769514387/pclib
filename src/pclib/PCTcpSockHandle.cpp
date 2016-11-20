#include "PC_Lib.h"
#include "PCUtilNetwork.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpPoller.h"
#include "PCTcpSockHandle.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

CPCTcpSockHandle::CPCTcpSockHandle(eSockType eType) :
	m_SocketFd(PC_INVALID_SOCKET),
	m_ListenSocketFd(PC_INVALID_SOCKET),
	m_SocketType(eType),
	m_SendBuffer(PER_SOCK_REQBUF_SIZE),
	m_RecvBuffer(PER_SOCK_REQBUF_SIZE),
	m_ActualSendedLen(0)
{
	PC_ASSERT((eType == eSockType::eAcceptType || eType == eSockType::eConnectType || eType == eSockType::eListenType), "eType (%d) error!", eType);
    Cleanup();
#if defined (_WIN32)
#else
	m_pPollerThread =  CPCTcpPoller::GetInstance()->GetPollerThread();
#endif
}

CPCTcpSockHandle::~CPCTcpSockHandle()
{
    Cleanup();
}

bool CPCTcpSockHandle::Create(int nPort, bool bBlock)
{
	//�����׽���
	Cleanup();
	m_SocketFd = PCCreateTcpSocket(nPort, bBlock);
	if (PC_INVALID_SOCKET == m_SocketFd)
	{
		return false;
	}

#if defined (_WIN32)
	//����ɶ˿�
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_SocketFd, (ULONG_PTR)this))
	{
		PCCloseSocket(m_SocketFd);
		return false;
    }
#endif

	//����Ƿ�����׽��ֻ�Ҫ��ʼ����
	if (nPort >= 0 && nPort <= 65535)
	{
		if (listen(m_SocketFd, SOMAXCONN) != 0)
		{
            PC_ERROR_LOG("listen(nPort=%d) fail! errno=%d", nPort, PCGetLastError(true));
			PCCloseSocket(m_SocketFd);
			return false;
        }

#if defined (_WIN32)
		m_Opt = eOpt::eAccept;
#else
        if(false == EpollEventCtl(EPOLLIN | EPOLLERR  , EPOLL_CTL_ADD))
        {
            PC_ERROR_LOG("listen(port=%d) EpollEventCtl fail!", nPort);
            PCCloseSocket(m_SocketFd);
            return false;
        }
#endif
        m_ListenSocketFd = m_SocketFd;
		PC_TRACE_LOG("listen(nPort=%d) ok!", nPort);
	}
	return true;
}

void CPCTcpSockHandle::Cleanup(bool bGracefully)
{
#if defined (_WIN32)
	m_Opt = eOpt::eUnconnect;
    ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
#else
    m_events = EPOLLIN | EPOLLOUT |  EPOLLERR | EPOLLHUP | EPOLLET;
    m_epctlOp = EPOLL_CTL_DEL;
#endif

	if (m_SocketFd != PC_INVALID_SOCKET)
	{
#if defined (_WIN32)
#else
        EpollEventCtl(EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET, EPOLL_CTL_DEL);
#endif
		if (bGracefully)
		{
			PCShutdownSocket(m_SocketFd);
			m_SocketFd = PC_INVALID_SOCKET;
		}
		else
		{
			PCCloseSocket(m_SocketFd);
		}
	}
	memset(m_pszRemoteIP, 0, sizeof(m_pszRemoteIP));
	m_ActualSendedLen = 0;
	m_RecvBuffer.Reset(0);
	m_RecvBuffer.Reset(0);
}

bool CPCTcpSockHandle::PostConnect(const char *pszHostAddress, int nPort)
{
	if (PC_INVALID_SOCKET == m_SocketFd || pszHostAddress == NULL || nPort < 0 || nPort > 65535)
	{
		PC_ERROR_LOG("m_SocketFd == PC_INVALID_SOCKET || pszHostAddress = %s || nPort = %d ", pszHostAddress, nPort);
		return false;
	}

#if defined (_WIN32)

	//��ȡ��ַ�ṹ
	struct sockaddr_in RemoteAddr;
	int nRet = PCDnsParseAddrIPv4(pszHostAddress, nPort, &RemoteAddr);
	if (PC_RESULT_SUCCESS != nRet)
	{
		return false;
	}

	//����
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_Opt = eOpt::eConnect;

	BOOL rc = CPCLib::m_lpfnConnectEx(m_SocketFd, (const struct sockaddr FAR *)&RemoteAddr, sizeof(RemoteAddr), NULL, 0, NULL, &(m_ioCtx.m_olOriginal));
	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("m_lpfnConnectEx fail��nErrNo = %d", nErrNo);
			return false;
		}
	}
	return true;
#else

#endif
}

bool CPCTcpSockHandle::PostSend()
{
	if (PC_INVALID_SOCKET == m_SocketFd)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET��");
		return false;
	}

#if defined (_WIN32)
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_Opt = eOpt::eWrite;
	m_wsBufPointer.buf = m_SendBuffer.UnsafeBuffer() + m_ActualSendedLen;
	m_wsBufPointer.len = static_cast<unsigned long>(m_SendBuffer.Size() - m_ActualSendedLen);

	DWORD dwSendBytes;
	int rc = WSASend(m_SocketFd, &(m_wsBufPointer), 1, &dwSendBytes, 0, &(m_ioCtx.m_olOriginal), NULL);
	if (rc != 0)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("WSASend fail��nErrNo = %d", nErrNo);
			return false;
		}
	}

	return true;
#else
	if(false == EpollEventCtl(EPOLLOUT | EPOLLET, EPOLL_CTL_ADD))
	{
		return false;
	}
#endif
}

bool CPCTcpSockHandle::PostRecv()
{
	if (PC_INVALID_SOCKET == m_SocketFd)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET��");
		return false;
	}

#if defined (_WIN32)
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_Opt = eOpt::eRead;
	m_RecvBuffer.Reset(0);
	m_wsBufPointer.buf = m_RecvBuffer.UnsafeBuffer();
	m_wsBufPointer.len = PER_SOCK_REQBUF_SIZE;

	DWORD dwRecvedSize = 0;
	DWORD dwFlags = 0;

	int rc = WSARecv(m_SocketFd,
		&(m_wsBufPointer),
		1,
		&dwRecvedSize,
		&dwFlags,
		&(m_ioCtx.m_olOriginal),
		NULL);
	if (rc != 0)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("WSARecv fail��nErrNo = %d", nErrNo);
			return false;
		}
    }
#else
    if(false == EpollEventCtl(EPOLLIN | EPOLLET, EPOLL_CTL_ADD))
    {
        return false;
    }
#endif

    return true;
}

bool CPCTcpSockHandle::PostAccept(PC_SOCKET sListen)
{
	if (PC_INVALID_SOCKET == sListen)
	{
		PC_ERROR_LOG("sListen == PC_INVALID_SOCKET��");
		return false;
	}
	m_ListenSocketFd = sListen;

#if defined (_WIN32)

	//����˸���һ��accept�Ŀͻ�Ԥ�ȴ���socket
	Cleanup();
	m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_SocketFd == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("socket fail ��WSAGetLastError() = %ld", WSAGetLastError());
		return false;
	}

	// �����׽���Ϊ������ģʽ
	if (PCSetNonBlocking(m_SocketFd) != 0)
	{
		PCCloseSocket(m_SocketFd);
		return false;
	}

	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_Opt = eOpt::eAccept;
	m_RecvBuffer.Reset(0);
	m_ioCtx.m_pOwner = this;

	DWORD dwRecvedSize = 0;
	BOOL rc = CPCLib::m_lpfnAcceptEx(
		sListen,
		m_SocketFd,
		m_RecvBuffer.UnsafeBuffer(),
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&dwRecvedSize,
		&(m_ioCtx.m_olOriginal));

	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("AcceptEx fail�� nErrNo = %d", nErrNo);
			PCCloseSocket(m_SocketFd);
			return false;
		}
	}
#else
	CPCTcpPoller::GetInstance()->BindAcceptHandle(this);
#endif
	PC_TRACE_LOG("PostAccept success! m_ListenSocketFd = %d", m_ListenSocketFd);
	return true;
}


void CPCTcpSockHandle::ProcessAccept()
{
	CPCGuard guard(m_Mutex);
	m_RecvBuffer.Reset(0);

#if defined (_WIN32)
	//״̬У��
	if (m_Opt != eOpt::eAccept)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eAccept);
		return ProcessClose();
	}

	//��SOCKET����ɶ˿ڽ��й���
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_SocketFd, (ULONG_PTR)this))
	{
		return ProcessClose();
	}

	//������ַ
	SOCKADDR_IN* addrClient = NULL;
	SOCKADDR_IN* addrLocal = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	CPCLib::m_lpfnGetAcceptExSockAddrs((LPVOID)m_RecvBuffer.UnsafeBuffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &localLen, (LPSOCKADDR*)&addrClient, &remoteLen);
	if (NULL == inet_ntop(AF_INET, &addrClient->sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
		return ProcessClose();
	}
	
#else
    //��������
	sockaddr_in addrClient;
	memset(&addrClient, 0, sizeof(sockaddr_in));
	int addrClientlen = sizeof(addrClient);
	m_SocketFd = accept(m_ListenSocketFd, (sockaddr *)&addrClient, (socklen_t*)&addrClientlen);
	if (m_SocketFd == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("accept fail! errno=%d", PCGetLastError(true));
		return ProcessClose();
	}

    //��ȡ�Է���ַ
	if (NULL == inet_ntop(AF_INET, &addrClient.sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
		return ProcessClose();
	}

    // �������ӵ��׽���Ϊ������ģʽ
    if (PCSetNonBlocking(m_SocketFd) != 0)
    {
        return ProcessClose();
    }
#endif

	PC_TRACE_LOG("accept(%s) client.", m_pszRemoteIP);
	return OnAccepted();
}

void CPCTcpSockHandle::ProcessConnect()
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//״̬У��
	if (m_Opt != eOpt::eConnect)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eConnect);
		return ProcessClose();
	}
	OnConnected();
#else

#endif
}

void CPCTcpSockHandle::ProcessSend(unsigned long dwSendedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//״̬У��
	if (m_Opt != eOpt::eWrite)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eWrite);
		return ProcessClose();
	}

	//�ж��Ƿ񶼷�����
	m_ActualSendedLen += dwSendedLen;
	if (m_ActualSendedLen < m_SendBuffer.Size() )
	{
		//û���� ע�ᷢ������ȴ��´η���
		if (!PostSend())
		{
			return ProcessClose();
		}
		return;
	}
#else
	while(1)
	{
		int nWaitSendLen = m_SendBuffer.Size() - m_ActualSendedLen;
		if(nWaitSendLen <= 0)
		{
			//������
			break;
		}

		int nSendedLen = send(m_SocketFd, m_SendBuffer.UnsafeBuffer() + m_ActualSendedLen, nWaitSendLen, 0);
		if (nSendedLen >= 0)
		{
			m_ActualSendedLen += nSendedLen;
			PC_TRACE_LOG("send succ. nWaitSendLen = %d, nSendedLen = %d ,m_ActualSendedLen = %lu", nWaitSendLen, nSendedLen, m_ActualSendedLen);
		}
		else
		{
			int errNo = PCGetLastError();
			if (errNo == EAGAIN || errNo == EWOULDBLOCK)
			{
				//û���� ���ͻ�����������ע�ᷢ������ȴ��´η���
				if (!PostSend())
				{
					return ProcessClose();
				}
				return;
			}
			else
			{
				//���ͳ��ִ���
				PC_ERROR_LOG("other err. send fail. errNo = %d", errNo);
				return ProcessClose();
			}
		}
	}
	
#endif
	
	//������
	PC_TRACE_LOG("send over:m_ActualSendedLen(%lu), m_SendBuffer.Size(%lu).", m_ActualSendedLen,m_SendBuffer.Size() );
	OnSendded();
}

void CPCTcpSockHandle::ProcessRecv(unsigned long dwRecvedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//״̬У��
	if (m_Opt != eOpt::eRead)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eRead);
		return ProcessClose();
	}
	//����windows����ʱ�����Ѿ������ˣ��յ��ĳ��ȼ���dwRecvedLen
#else
	//����linux����Ҫ�ֶ������ݣ������dwRecvedLenûʲô����
    m_RecvBuffer.Reset(0);
	dwRecvedLen = 0;
	while (1)
	{
        int nRecvdLen = recv(m_SocketFd, m_RecvBuffer.UnsafeBuffer() + dwRecvedLen, PER_SOCK_REQBUF_SIZE, 0);
		if (nRecvdLen > 0)
		{
			dwRecvedLen += nRecvdLen;
			PC_TRACE_LOG("recv end ,nRecvdLen = %d, dwRecvedLen = %d", nRecvdLen, dwRecvedLen);
		}
		else if (nRecvdLen == 0)
		{
            //���ӱ��Է��ر�
			PC_TRACE_LOG("recv = 0 conn closed. dwRecvedLen = %d",  dwRecvedLen);
            if(dwRecvedLen == 0)
            {
                //�������չ�����û���յ��κ����ݣ���ʾ�����ѱ��Ͽ�
                return ProcessClose();
            }
            else
            {
                //���յ��˲������ݣ�Ȼ�����ӱ��رա������Ƿ��������ɵ������ж�
                break;
            }
		}
		else
		{
			int errNo = PCGetLastError();
			if (errNo == EAGAIN || errNo == EWOULDBLOCK)
			{
				//���������ȫ�������Ѿ�ȫ������
				PC_TRACE_LOG("errNo == EAGAIN || errNo == EWOULDBLOCK,buf is read end.dwRecvedLen = %d",  dwRecvedLen);
				break;
			}
			else
			{
				//���ճ��ִ���
				PC_ERROR_LOG("other err. recv fail. errNo = %d", errNo);
				return ProcessClose();
			}
		}
	}
#endif

	//���ڽ���ʱʹ����unsafebuffer����ʱ��Ҫ�ѳ���Ҳ����ָ��
	m_RecvBuffer.Reset(dwRecvedLen);
	OnRecved(dwRecvedLen);
}

void CPCTcpSockHandle::ProcessClose()
{
	CPCGuard guard(m_Mutex);

	if (m_SocketType == eSockType::eAcceptType)
	{
		if (false == PostAccept(m_ListenSocketFd))
		{
			PC_WARN_LOG("warning! closed eAcceptType PostAccept fail.");
		}
	}

	Cleanup();
	OnClosed();
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
