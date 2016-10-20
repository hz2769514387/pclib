#include "PC_Lib.h"
#include "PCUtilNetwork.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCTcpConnection.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

CTcpConnection::CTcpConnection() :
	m_hTcpSocket(PC_INVALID_SOCKET),
	m_bListenSocket(false)
{
	Cleanup();
}

CTcpConnection::~CTcpConnection()
{
	Cleanup();
}

bool CTcpConnection::Create(const char * service, bool bBlock )
{
	Cleanup();
	m_hTcpSocket = PCCreateTcpSocket(service, bBlock);
	if (PC_INVALID_SOCKET != m_hTcpSocket)
	{
		return false;
	}
	if (NULL != service && 0 != service[0])
	{
		m_bListenSocket = true;
	}
	return true;
}

void CTcpConnection::Cleanup()
{
	PCShutdownSocket(m_hTcpSocket);
	m_bListenSocket = false;
	m_dwSendLen = 0;
	memset(m_szSendBuff, 0, sizeof(m_szSendBuff));
	memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
	memset(m_pszRemoteIP, 0, sizeof(m_pszRemoteIP));
}

bool CTcpConnection::DoAccept(unsigned long dwSize)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//将SOCKET与完成端口进行关联
	//if (!CIOCP::GetInstance()->AssociateSocketWithCompletionPort(m_hTcpSocket, (ULONG_PTR)this))
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//解析地址
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);
	CPCLib::m_lpfnGetAcceptExSockAddrs(m_szRecvBuff, MAX_SOCK_CACHE_SIZE - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
	if (NULL == inet_ntop(AF_INET, &ClientAddr->sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//如果有数据则处理数据，否则投递一个接受数据请求
	if (dwSize > 0)
	{
		DoRecved(dwSize);
		return true;
	}
	else
	{
		return PostRecv();
	}

#else
	
#endif
}



bool CTcpConnection::PostConnect(const char *pszHostAddress, const char * service)
{
	if (PC_INVALID_SOCKET == m_hTcpSocket || pszHostAddress == NULL || service == NULL)
	{
		PCCloseSocket(m_hTcpSocket);
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET || pszHostAddress == NULL || service == NULL ");
		return false;
	}

#if defined (_WIN32)
	//将SOCKET与完成端口进行关联
	//if (!CIOCP::GetInstance()->AssociateSocketWithCompletionPort(m_hTcpSocket, (ULONG_PTR)this))
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//获取地址结构
	struct sockaddr_in RemoteAddr;
	int nRet = PCDnsParseAddrIPv4(pszHostAddress, service, &RemoteAddr);
	if (PC_RESULT_SUCCESS != nRet)
	{
		PCCloseSocket(m_hTcpSocket);
		return false;
	}

	//连接
	m_overLapped.byOpCode = OP_CONNECT;
	ZeroMemory(&m_overLapped.ol, sizeof(m_overLapped.ol));
	BOOL bRet = CPCLib::m_lpfnConnectEx(m_hTcpSocket, (const struct sockaddr FAR *)&RemoteAddr, sizeof(RemoteAddr), NULL, 0, NULL, &m_overLapped.ol);
	if (!bRet)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PCCloseSocket(m_hTcpSocket);
			PC_ERROR_LOG("m_lpfnConnectEx失败！");
			return false;
		}
	}
	return true;
#else

#endif
}

bool CTcpConnection::PostSend()
{
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		PCCloseSocket(m_hTcpSocket);
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
		return false;
	}

#if defined (_WIN32)
	m_wsSendBuff.len = m_dwSendLen;
	m_wsSendBuff.buf = m_szSendBuff;
	DWORD dwSendBytes;
	ZeroMemory(&m_sendOverLapped.ol, sizeof(m_sendOverLapped.ol));
	m_sendOverLapped.byOpCode = OP_WRITE;
	int rc = WSASend(m_hTcpSocket, &m_wsSendBuff, 1, &dwSendBytes, 0, &m_sendOverLapped.ol, NULL);
	if (rc != 0)
	{
		if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
		{
			PCCloseSocket(m_hTcpSocket);
			PC_ERROR_LOG("WSASend失败！");
			return false;
		}
	}

	return true;
#else

#endif
}

bool CTcpConnection::PostRecv()
{
	if (PC_INVALID_SOCKET == m_hTcpSocket)
	{
		PCCloseSocket(m_hTcpSocket);
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
		return false;
	}

#if defined (_WIN32)
	m_wsBuff.buf = m_szRecvBuff;
	m_wsBuff.len = MAX_SOCK_CACHE_SIZE;
	DWORD dwRecvedSize = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&m_overLapped.ol, sizeof(m_overLapped.ol));
	m_overLapped.byOpCode = OP_READ;
	int rc = WSARecv(m_hTcpSocket,
		&m_wsBuff,
		1,
		&dwRecvedSize,
		&dwFlags,
		&m_overLapped.ol,
		NULL);
	if (rc != 0)
	{
		if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
		{
			PCCloseSocket(m_hTcpSocket);
			PC_ERROR_LOG("WSARecv失败！");
			return false;
		}
	}

	return true;
#else

#endif
}

bool CTcpConnection::PostAccept(PC_SOCKET sListen)
{
	if (PC_INVALID_SOCKET == sListen)
	{
		PCCloseSocket(m_hTcpSocket);
		PC_ERROR_LOG("sListen == PC_INVALID_SOCKET！");
		return false;
	}
#if defined (_WIN32)

	//服务端给下一次accept的客户预先创建socket
	Cleanup();
	m_hTcpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hTcpSocket == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("CIOCPTcpSocket::CreateSocket socket 失败！WSAGetLastError() = %ld", WSAGetLastError());
		return false;
	}

	// 设置套接字为非阻塞模式
	if (PCSetNonBlocking(m_hTcpSocket) != 0)
	{
		PCCloseSocket(m_hTcpSocket);
		PC_ERROR_LOG("CIOCPTcpSocket::CreateSocket 设置套接字为非阻塞模式 失败！");
		return false;
	}

	ZeroMemory(&m_overLapped.ol, sizeof(m_overLapped.ol));
	m_overLapped.byOpCode = OP_ACCEPT;
	m_overLapped.hClient = this;
	DWORD dwRecvedSize = 0;

	BOOL rc = CPCLib::m_lpfnAcceptEx(
		sListen,
		m_hTcpSocket,
		m_szRecvBuff,
		MAX_SOCK_CACHE_SIZE - ((sizeof(sockaddr_in) + 16) * 2),
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		&dwRecvedSize,
		(OVERLAPPED *)&m_overLapped);

	if (!rc)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PCCloseSocket(m_hTcpSocket);
			PC_ERROR_LOG("AcceptEx失败！");
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
