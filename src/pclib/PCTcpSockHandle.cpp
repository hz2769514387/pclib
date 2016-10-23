#include "PC_Lib.h"
#include "PCUtilNetwork.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpPoller.h"
#include "PCTcpSockHandle.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

CPCTcpSockHandle::CPCTcpSockHandle() :
	m_hTcpSocket(PC_INVALID_SOCKET),
	m_bListenSocket(false)
{
	Cleanup();
}

CPCTcpSockHandle::~CPCTcpSockHandle()
{
	Cleanup();
}

bool CPCTcpSockHandle::Create(int nPort, bool bBlock)
{
	//�����׽���
	Cleanup();
	m_hTcpSocket = PCCreateTcpSocket(nPort, bBlock);
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		return false;
	}

#if defined (_WIN32)
	//����ɶ˿�
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_hTcpSocket, (ULONG_PTR)this))
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}
#else
	//����epoll���
	m_epollFd = CPCTcpPoller::GetInstance()->GetEpollFd();
	if (PC_INVALID_SOCKET == m_epollFd)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}
#endif

	//����Ƿ�����׽��ֻ�Ҫ��ʼ����
	if (nPort >= 0 && nPort <= 65535)
	{
		if (listen(m_hTcpSocket, SOMAXCONN) != 0)
		{
			PC_ERROR_LOG("listen(m_hTcpSocket=%d) fail! errno=%d", m_hTcpSocket, PCGetLastError(true));
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
		m_bListenSocket = true;
	}
	return true;
}

void CPCTcpSockHandle::Cleanup()
{
	PCShutdownSocket(m_hTcpSocket);
	m_bListenSocket = false;
	memset(m_pszRemoteIP, 0, sizeof(m_pszRemoteIP));
}

bool CPCTcpSockHandle::DoAccept(bool bSucceed, const char *szRecvedBuff, unsigned long dwRecvedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//��SOCKET����ɶ˿ڽ��й���
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_hTcpSocket, (ULONG_PTR)this))
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//������ַ
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	CPCLib::m_lpfnGetAcceptExSockAddrs((LPVOID)szRecvedBuff, PER_SOCK_REQBUF_SIZE - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
	if (NULL == inet_ntop(AF_INET, &ClientAddr->sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
		PCCloseSocket(m_hTcpSocket);
		return false;
	}
	PC_TRACE_LOG("accept(%s) client.", m_pszRemoteIP);

	//����������������ݣ�����Ͷ��һ��������������
	if (dwRecvedLen > 0)
	{
		DoRecved(true, szRecvedBuff, dwRecvedLen);
		return true;
	}
	else
	{
		return PostRecv();
	}

#else
	
#endif
}

bool CPCTcpSockHandle::PostConnect(const char *pszHostAddress, int nPort)
{
	if (PC_INVALID_SOCKET == m_hTcpSocket || pszHostAddress == NULL || nPort < 0 || nPort > 65535)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET || pszHostAddress = %s || nPort = %d ", pszHostAddress, nPort);
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

#if defined (_WIN32)

	//��ȡ��ַ�ṹ
	struct sockaddr_in RemoteAddr;
	int nRet = PCDnsParseAddrIPv4(pszHostAddress, nPort, &RemoteAddr);
	if (PC_RESULT_SUCCESS != nRet)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//����
	IOCP_IO_CTX* locpCtx = new IOCP_IO_CTX(OP_CONNECT, this);
	BOOL rc = CPCLib::m_lpfnConnectEx(m_hTcpSocket, (const struct sockaddr FAR *)&RemoteAddr, sizeof(RemoteAddr), NULL, 0, NULL, &(locpCtx->m_olOriginal));
	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("m_lpfnConnectEx fail��nErrNo = %d", nErrNo);
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
	}
	return true;
#else

#endif
}

bool CPCTcpSockHandle::PostSend(const char *szSendBuff, unsigned long nSendLen)
{
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET��");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

#if defined (_WIN32)
	IOCP_IO_CTX* locpCtx = new IOCP_IO_CTX(OP_WRITE, this, szSendBuff, nSendLen);
	DWORD dwSendBytes;
	int rc = WSASend(m_hTcpSocket, &(locpCtx->m_wsBufPointer), 1, &dwSendBytes, 0, &(locpCtx->m_olOriginal), NULL);
	if (rc != 0)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("WSASend fail��nErrNo = %d", nErrNo);
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
	}

	return true;
#else

#endif
}

bool CPCTcpSockHandle::PostRecv()
{
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET��");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

#if defined (_WIN32)
	IOCP_IO_CTX* locpCtx = new IOCP_IO_CTX(OP_READ, this);
	DWORD dwRecvedSize = 0;
	DWORD dwFlags = 0;

	int rc = WSARecv(m_hTcpSocket,
		&(locpCtx->m_wsBufPointer),
		1,
		&dwRecvedSize,
		&dwFlags,
		&(locpCtx->m_olOriginal),
		NULL);
	if (rc != 0)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("WSARecv fail��nErrNo = %d", nErrNo);
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
	}
	return true;
#else

#endif
}

bool CPCTcpSockHandle::PostAccept(PC_SOCKET sListen)
{
	if (PC_INVALID_SOCKET == sListen)
	{
		PC_ERROR_LOG("sListen == PC_INVALID_SOCKET��");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}
#if defined (_WIN32)

	//����˸���һ��accept�Ŀͻ�Ԥ�ȴ���socket
	Cleanup();
	m_hTcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hTcpSocket == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("socket fail ��WSAGetLastError() = %ld", WSAGetLastError());
		return false;
	}

	// �����׽���Ϊ������ģʽ
	if (PCSetNonBlocking(m_hTcpSocket) != 0)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	IOCP_IO_CTX* locpCtx = new IOCP_IO_CTX(OP_ACCEPT, this);
	DWORD dwRecvedSize = 0;

	BOOL rc = CPCLib::m_lpfnAcceptEx(
		sListen,
		m_hTcpSocket,
		locpCtx->m_szIOBuf,
		PER_SOCK_REQBUF_SIZE - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&dwRecvedSize,
		&(locpCtx->m_olOriginal));

	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("AcceptEx fail�� nErrNo = %d", nErrNo);
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
	}
	return true;
#else

#endif
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
