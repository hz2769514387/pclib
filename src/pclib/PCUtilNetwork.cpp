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

PC_SOCKET  PCCreateTcpSocket(const char * service, bool blocked)
{
	//配置地址信息  
	struct addrinfo addrLocal;
	memset(&addrLocal, 0, sizeof(addrLocal));
	addrLocal.ai_family = PC_SOCKET_TYPE;
	addrLocal.ai_socktype = SOCK_STREAM;
	addrLocal.ai_protocol = IPPROTO_TCP;
	if (NULL != service && 0 != service[0])
	{
		addrLocal.ai_flags = AI_PASSIVE;
	}	

	//获取地址
	struct addrinfo *addrResults = NULL;
	int nRet = getaddrinfo(NULL, service, &addrLocal, &addrResults);
	if (nRet != 0)
	{
		PC_ERROR_LOG("getaddrinfo(%s)  fail. nRet = %d(%s), errno = %d", service, nRet, gai_strerrorA(nRet), PCGetLastError(true));
		return PC_INVALID_SOCKET;
	}

	//根据地址结构进行socket创建和绑定
	struct addrinfo *addr = addrResults;
	PC_SOCKET sockFd = PC_INVALID_SOCKET;
	while (addr != NULL)
	{
		//建立socket  
		sockFd = socket(addrLocal.ai_family, addrLocal.ai_socktype, addrLocal.ai_protocol);
		if (PC_INVALID_SOCKET != sockFd)
		{
			if (!blocked)
			{
				// 设置套接字为非阻塞模式
				if (PCSetNonBlocking(sockFd) != PC_RESULT_SUCCESS)
				{
					PCCloseSocket(sockFd);
					addr = addr->ai_next;
					continue;
				}
			}

			if (NULL != service && 0 != service[0])
			{
				//服务端套接字需要执行绑定监听
				if ((bind(sockFd, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == 0) && listen(sockFd, SOMAXCONN) == 0)
				{
					//成功
					break;
				}
			}
			else
			{
				//客户端套接字只需要执行绑定
				if (bind(sockFd, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == 0)
				{
					//成功
					break;
				}
			}
			PCCloseSocket(sockFd);
		}
		PC_WARN_LOG("use socket fail. try next. errno = %d", PCGetLastError(true));
		addr = addr->ai_next;
	}
	freeaddrinfo(addrResults);
	return sockFd;	
}

int PCDnsParseAddrIPv4(const char *hostname4, const char * service, struct sockaddr_in *addr4)
{
	if (hostname4 == NULL || hostname4[0] == 0 || service == NULL || addr4 == NULL)
	{
		PC_ERROR_LOG("params err! hostname4 = %s", hostname4);
		return PC_RESULT_PARAM;
	}
	addr4->sin_family = AF_INET;
	addr4->sin_port = htons(atoi(service));
	memset(addr4->sin_zero, 0, sizeof(addr4->sin_zero));

	//假设hostname4是一个点分十进制，无需DNS解析，直接转换
	int nRet = inet_pton(addr4->sin_family, hostname4, (void*)(&(addr4->sin_addr)));
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
	struct addrinfo *addrServer;
	nRet = getaddrinfo(hostname4, service, &addrHint, &addrServer);
	if (nRet != 0 || addrServer == NULL)
	{
		PC_ERROR_LOG("getaddrinfo(%s:%s)  fail. nRet = %d(%s), errno = %d", hostname4, service, nRet, gai_strerrorA(nRet), PCGetLastError(true));
		return PC_RESULT_SYSERROR;
	}

	addr4->sin_addr.s_addr = ((struct sockaddr_in  *)(addrServer->ai_addr))->sin_addr.s_addr;
	freeaddrinfo(addrServer);
	return PC_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
