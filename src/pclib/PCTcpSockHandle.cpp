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
	CPCTcpPoller::GetInstance()->BindTcpSockHandle(this);
#if defined (_WIN32)
#else
	m_pPollerThread =  CPCTcpPoller::GetInstance()->GetPollerThread();
#endif
}

CPCTcpSockHandle::~CPCTcpSockHandle()
{
	Cleanup();
	CPCTcpPoller::GetInstance()->UnBindTcpSockHandle(this);
}

bool CPCTcpSockHandle::Create(int nPort, bool bBlock)
{
	//创建套接字
	Cleanup();
	m_SocketFd = PCCreateTcpSocket(nPort, bBlock);
	if (PC_INVALID_SOCKET == m_SocketFd)
	{
		return false;
	}

#if defined (_WIN32)
	//绑定完成端口
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_SocketFd, (ULONG_PTR)this))
	{
		PCCloseSocket(m_SocketFd);
		return false;
    }
#endif

	//如果是服务端套接字还要开始监听
	if (nPort >= 0 && nPort <= 65535)
	{
		if (listen(m_SocketFd, SOMAXCONN) != 0)
		{
			PC_ERROR_LOG("listen(m_SocketFd=%d) fail! errno=%d", m_SocketFd, PCGetLastError(true));
			PCCloseSocket(m_SocketFd);
			return false;
		}

#if defined (_WIN32)
		m_Opt = eOpt::eAccept;
#else
		//epoll_ctl请求处理
		struct epoll_event epv ;
		memset(&epv, 0, sizeof(epv));
		epv.events = EPOLLIN | EPOLLOUT | EPOLLERR;
		if (0 != epoll_ctl(m_pPollerThread->m_epollFd, EPOLL_CTL_ADD, m_SocketFd, &epv))
		{
			PC_WARN_LOG("Create listen epoll_ctl fail.socket fd = %d, op = %d, events = %d", m_SocketFd, m_epctlOp, m_events);
		}
#endif
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
	m_events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET;
	m_epctlOp = EPOLL_CTL_DEL;
#endif

	if (m_SocketFd != PC_INVALID_SOCKET)
	{
#if defined (_WIN32)
#else
		//epoll_ctl请求处理
		struct epoll_event epv ;
		memset(&epv, 0, sizeof(epv));
		epv.events = m_events;
		epv.data.fd = m_SocketFd;
		epv.data.ptr = this;
		if (0 != epoll_ctl(m_pPollerThread->m_epollFd, m_epctlOp, m_SocketFd, &epv))
		{
			PC_WARN_LOG("Create listen epoll_ctl fail.socket fd = %d, op = %d, events = %d", m_SocketFd, m_epctlOp, m_events);
		}
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

	//获取地址结构
	struct sockaddr_in RemoteAddr;
	int nRet = PCDnsParseAddrIPv4(pszHostAddress, nPort, &RemoteAddr);
	if (PC_RESULT_SUCCESS != nRet)
	{
		return false;
	}

	//连接
	ZeroMemory(&m_ioCtx.m_olOriginal, sizeof(m_ioCtx.m_olOriginal));
	m_Opt = eOpt::eConnect;

	BOOL rc = CPCLib::m_lpfnConnectEx(m_SocketFd, (const struct sockaddr FAR *)&RemoteAddr, sizeof(RemoteAddr), NULL, 0, NULL, &(m_ioCtx.m_olOriginal));
	if (!rc)
	{
		int nErrNo = WSAGetLastError();
		if (nErrNo != WSA_IO_PENDING)
		{
			PC_ERROR_LOG("m_lpfnConnectEx fail！nErrNo = %d", nErrNo);
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
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
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
			PC_ERROR_LOG("WSASend fail！nErrNo = %d", nErrNo);
			return false;
		}
	}

	return true;
#else

#endif
}

bool CPCTcpSockHandle::PostRecv()
{
	if (PC_INVALID_SOCKET == m_SocketFd)
	{
		PC_ERROR_LOG("m_hTcpSocket == PC_INVALID_SOCKET！");
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
			PC_ERROR_LOG("WSARecv fail！nErrNo = %d", nErrNo);
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
		return false;
	}
	m_ListenSocketFd = sListen;

#if defined (_WIN32)

	//服务端给下一次accept的客户预先创建socket
	Cleanup();
	m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_SocketFd == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("socket fail ！WSAGetLastError() = %ld", WSAGetLastError());
		return false;
	}

	// 设置套接字为非阻塞模式
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
			PC_ERROR_LOG("AcceptEx fail！ nErrNo = %d", nErrNo);
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
	//状态校验
	if (m_Opt != eOpt::eAccept)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eAccept);
		return ProcessClose();
	}

	//将SOCKET与完成端口进行关联
	if (!CPCTcpPoller::GetInstance()->AssociateSocketWithIOCP(m_SocketFd, (ULONG_PTR)this))
	{
		return ProcessClose();
	}

	//解析地址
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
	sockaddr_in addrClient;
	memset(&addrClient, 0, sizeof(sockaddr_in));
	int addrClientlen = sizeof(addrClient);

	m_SocketFd = accept(m_ListenSocketFd, (sockaddr *)&addrClient, (socklen_t*)&addrClientlen);
	if (m_SocketFd == PC_INVALID_SOCKET)
	{
		PC_ERROR_LOG("accept fail! errno=%d", PCGetLastError(true));
		return ProcessClose();
	}
	if (NULL == inet_ntop(AF_INET, &addrClient.sin_addr, m_pszRemoteIP, sizeof(m_pszRemoteIP)))
	{
		PC_ERROR_LOG("inet_ntop fail! errno=%d", PCGetLastError(true));
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
	//状态校验
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
	//状态校验
	if (m_Opt != eOpt::eWrite)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eWrite);
		return ProcessClose();
	}

	//判断是否都发完了
	m_ActualSendedLen += dwSendedLen;
	if (m_SendBuffer.Size() > m_ActualSendedLen)
	{
		//没发完
		if (!PostSend())
		{
			return ProcessClose();
		}
	}
	if (m_SendBuffer.Size() < m_ActualSendedLen)
	{
		//发多了
		PC_WARN_LOG("need send len:m_SendBuffer.Size()(%lu) < actual sended len :m_ActualSendedLen(%lu) , strange.", m_SendBuffer.Size(), m_ActualSendedLen);
	}
	OnSendded();
#else

#endif
}

void CPCTcpSockHandle::ProcessRecv(unsigned long dwRecvedLen)
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	//状态校验
	if (m_Opt != eOpt::eRead)
	{
		PC_ERROR_LOG("opt code :%d err! correct opt code is : %d", m_Opt, eOpt::eRead);
		return ProcessClose();
	}
	//由于接收时使用了unsafebuffer，此时需要把长度也补充指定
	m_RecvBuffer.Reset(dwRecvedLen);
	OnRecved(dwRecvedLen);
#else

#endif
}

void CPCTcpSockHandle::ProcessClose()
{
	CPCGuard guard(m_Mutex);

#if defined (_WIN32)
	if (m_SocketType == eSockType::eAcceptType)
	{
		if (false == PostAccept(m_ListenSocketFd))
		{
			PC_WARN_LOG("warning! closed eAcceptType PostAccept fail.");
		}
	}
#endif
	Cleanup();
	OnClosed();
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
