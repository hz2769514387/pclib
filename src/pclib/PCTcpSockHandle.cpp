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
#if defined (_WIN32)
#else
    m_epollFd = CPCTcpPoller::GetInstance()->GetEpollFd();
    m_eventFd = CPCTcpPoller::GetInstance()->GetEventFd();
#endif
	Cleanup();
}

CPCTcpSockHandle::~CPCTcpSockHandle()
{
	Cleanup();
}

bool CPCTcpSockHandle::Create(int nPort, bool bBlock)
{
	//创建套接字
	Cleanup();
	m_hTcpSocket = PCCreateTcpSocket(nPort, bBlock);
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		return false;
	}

#if defined (_WIN32)
	//绑定完成端口
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_hTcpSocket, (ULONG_PTR)this))
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
    }
#endif

	//如果是服务端套接字还要开始监听
	if (nPort >= 0 && nPort <= 65535)
	{
		if (listen(m_hTcpSocket, SOMAXCONN) != 0)
		{
			PC_ERROR_LOG("listen(m_hTcpSocket=%d) fail! errno=%d", m_hTcpSocket, PCGetLastError(true));
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
		m_bListenSocket = true;
		m_ConnOpt = csEnumOpt::cseAccept;
	}
	return true;
}

void CPCTcpSockHandle::Cleanup()
{
	PCShutdownSocket(m_hTcpSocket);
	m_hTcpSocket = PC_INVALID_SOCKET;
	m_bListenSocket = false;
	memset(m_pszRemoteIP, 0, sizeof(m_pszRemoteIP));

#if defined (_WIN32)
	m_ConnOpt = csEnumOpt::cseUnconnect;
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	memset(m_szIOBuf, 0, sizeof(m_szIOBuf));
	m_dwIOBufLen = 0;
	m_wsBufPointer.buf = m_szIOBuf;
	m_wsBufPointer.len = m_dwIOBufLen;
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

	//获取地址结构
	struct sockaddr_in RemoteAddr;
	int nRet = PCDnsParseAddrIPv4(pszHostAddress, nPort, &RemoteAddr);
	if (PC_RESULT_SUCCESS != nRet)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//连接
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_ConnOpt = csEnumOpt::cseConnect;

	BOOL rc = CPCLib::m_lpfnConnectEx(m_hTcpSocket, (const struct sockaddr FAR *)&RemoteAddr, sizeof(RemoteAddr), NULL, 0, NULL, &(m_ioCtx.m_olOriginal));
	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("m_lpfnConnectEx fail！nErrNo = %d", nErrNo);
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
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

#if defined (_WIN32)
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_ConnOpt = csEnumOpt::cseWrite;
	if (nSendLen <= PER_SOCK_REQBUF_SIZE)
	{
		memcpy(m_szIOBuf, szSendBuff, nSendLen);
		m_dwIOBufLen = nSendLen;
	}
	else
	{
		PC_WARN_LOG("dwIOBufLen(%ld) > %d, only copy(%d) length.", nSendLen, PER_SOCK_REQBUF_SIZE, PER_SOCK_REQBUF_SIZE);
		memcpy(m_szIOBuf, szSendBuff, PER_SOCK_REQBUF_SIZE);
		m_dwIOBufLen = PER_SOCK_REQBUF_SIZE;
	}
	m_wsBufPointer.buf = m_szIOBuf;
	m_wsBufPointer.len = m_dwIOBufLen;

	DWORD dwSendBytes;
	int rc = WSASend(m_hTcpSocket, &(m_wsBufPointer), 1, &dwSendBytes, 0, &(m_ioCtx.m_olOriginal), NULL);
	if (rc != 0)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("WSASend fail！nErrNo = %d", nErrNo);
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
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

#if defined (_WIN32)
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_ConnOpt = csEnumOpt::cseRead;
	m_wsBufPointer.buf = m_szIOBuf;
	m_wsBufPointer.len = PER_SOCK_REQBUF_SIZE;

	DWORD dwRecvedSize = 0;
	DWORD dwFlags = 0;

	int rc = WSARecv(m_hTcpSocket,
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
			PC_ERROR_LOG("WSARecv fail！nErrNo = %d", nErrNo);
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
		PC_ERROR_LOG("sListen == PC_INVALID_SOCKET！");
		PCCloseSocket(m_hTcpSocket);
		return false;
	}
#if defined (_WIN32)

	//服务端给下一次accept的客户预先创建socket
	Cleanup();
	m_hTcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hTcpSocket == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("socket fail ！WSAGetLastError() = %ld", WSAGetLastError());
		return false;
	}

	// 设置套接字为非阻塞模式
	if (PCSetNonBlocking(m_hTcpSocket) != 0)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_ConnOpt = csEnumOpt::cseAccept;
	m_ioCtx.m_pOwner = this;

	DWORD dwRecvedSize = 0;
	BOOL rc = CPCLib::m_lpfnAcceptEx(
		sListen,
		m_hTcpSocket,
		m_szIOBuf,
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
			PC_ERROR_LOG("AcceptEx fail！ nErrNo = %d", nErrNo);
			PCCloseSocket(m_hTcpSocket);
			return false;
		}
	}
	return true;
#else

#endif
}


void CPCTcpSockHandle::ProcessAccept()
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//将SOCKET与完成端口进行关联
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_hTcpSocket, (ULONG_PTR)this))
	{
		return ProcessClose();
	}

	//解析地址
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	CPCLib::m_lpfnGetAcceptExSockAddrs((LPVOID)m_szIOBuf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
	if (NULL == inet_ntop(AF_INET, &ClientAddr->sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
		return ProcessClose();
	}
	PC_TRACE_LOG("accept(%s) client.", m_pszRemoteIP);
	return OnAccepted();
#else

#endif
}

void CPCTcpSockHandle::ProcessConnect()
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)

#else

#endif
}

void CPCTcpSockHandle::ProcessSend(unsigned long dwSendedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	OnSendded(dwSendedLen);
#else

#endif
}

void CPCTcpSockHandle::ProcessRecv(unsigned long dwRecvedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	OnRecved(dwRecvedLen);
#else

#endif
}

void CPCTcpSockHandle::ProcessClose()
{
	CPCGuard guard(m_Mutex);

	Cleanup();
	OnClosed();
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
