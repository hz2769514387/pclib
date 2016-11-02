#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCUtilNetwork.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

int  PCSetNonBlocking(PC_SOCKET  sock)
{
#if defined (_WIN32)
	unsigned long ulArgp = 1;
	int nRet = ioctlsocket(sock, FIONBIO, &ulArgp);
	if (PC_SOCKET_ERROR == nRet)
	{
		PC_ERROR_LOG("ioctlsocket fail, errno = %d.", PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}
#else
	int opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		PC_ERROR_LOG("fcntl F_GETFL fail, errno = %d.", PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		PC_ERROR_LOG("fcntl F_SETFL fail, errno = %d.", PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}
#endif
	return PC_RESULT_SUCCESS;
}


int PCSockIsConnected(PC_SOCKET sock)
{
	char error[128] = {0};
	socklen_t len = sizeof(int);
#if defined (_WIN32)
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&error, &len) == PC_SOCKET_ERROR)
#else
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &len) == PC_SOCKET_ERROR)
#endif	
	{
		PC_ERROR_LOG("getsockopt == -1,socket is unconnected, errno = %d.", PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}
	return PC_RESULT_SUCCESS;
}

void PCShutdownSocket(PC_SOCKET sock)
{
	if (sock != PC_INVALID_SOCKET)
	{
		PC_SOCKET tempSocket = sock;
		sock = PC_INVALID_SOCKET;

		//SD_BOTH或SHUT_RDWR
		shutdown(tempSocket, 2);
		PCCloseSocket(tempSocket);
	}
}

PC_SOCKET  PCCreateTcpSocket(int nPort, bool blocked)
{
	//建立socket  
	PC_SOCKET sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (PC_INVALID_SOCKET == sockFd)
	{
		PC_ERROR_LOG("socket(AF_INET, SOCK_STREAM, 0) fail! errno=%d", PCGetLastError(true));
		return sockFd;
	}
	if (!blocked)
	{
		// 设置套接字为非阻塞模式
		if (PCSetNonBlocking(sockFd) != PC_RESULT_SUCCESS)
		{
			PCCloseSocket(sockFd);
			return PC_INVALID_SOCKET;
		}
	}

	//绑定地址
	struct sockaddr_in LocalAddr;
	memset(&LocalAddr, 0, sizeof(LocalAddr));
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = INADDR_ANY;
	LocalAddr.sin_port = htons((nPort >= 0 && nPort <= 65535)?nPort:0);
	if (bind(sockFd, (const sockaddr *)&LocalAddr, sizeof(struct sockaddr)) != 0)
	{
		PC_ERROR_LOG("bind(sockFd=%d) fail! errno=%d", sockFd, PCGetLastError(true));
		PCCloseSocket(sockFd);
		return PC_INVALID_SOCKET;
	}
	return sockFd;	
}

int PCDnsParseAddrIPv4(const char *pszHost, int nPort, struct sockaddr_in *addr4)
{
	if (pszHost == NULL || pszHost[0] == 0 || nPort < 0 || nPort > 65535)
	{
		PC_ERROR_LOG("params err! pszHost = %s, nPort = %d", pszHost, nPort);
		return PC_RESULT_PARAM;
	}
	addr4->sin_family = AF_INET;
	addr4->sin_port = htons(nPort);
	memset(addr4->sin_zero, 0, sizeof(addr4->sin_zero));

	//假设hostname4是一个点分十进制，无需DNS解析，直接转换
	int nRet = inet_pton(addr4->sin_family, pszHost, (void*)(&(addr4->sin_addr)));
	if (nRet > 0)
	{
		return PC_RESULT_SUCCESS;
	}

	//直接分析失败，进行DNS解析
	struct addrinfo addrHint;
	memset(&addrHint, 0, sizeof(addrHint));
	addrHint.ai_family = addr4->sin_family;
	addrHint.ai_socktype = SOCK_STREAM;
	addrHint.ai_protocol = IPPROTO_TCP;
	struct addrinfo *addrServer = NULL;

	char szService[11] = { 0 };
	sprintf(szService, "%d", nPort);
	nRet = getaddrinfo(pszHost, szService, &addrHint, &addrServer);
	if (nRet != 0 || addrServer == NULL)
	{
        PC_ERROR_LOG("getaddrinfo(%s:%d)  fail. nRet = %d(%s), errno = %d", pszHost, nPort, nRet, gai_strerror(nRet), PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}

	addr4->sin_addr.s_addr = ((struct sockaddr_in  *)(addrServer->ai_addr))->sin_addr.s_addr;
	freeaddrinfo(addrServer);
	return PC_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
