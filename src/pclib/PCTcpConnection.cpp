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
	GetAcceptExSockaddrs(m_szRecvBuff, MAX_SOCK_CACHE_SIZE - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);
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

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
