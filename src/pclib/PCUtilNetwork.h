#if !defined(_PCUTILNETWORK_H_)
#define _PCUTILNETWORK_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		设置SOCKET连接为非阻塞模式
*@param		sock	[IN]	套接字
*@return	成功为0，失败为错误码，见PC_Lib.h
*/
int  PCSetNonBlocking(PC_SOCKET sock);

/**
*@brief		判断SOCKET连接是否已连接上
*@param		sock	[IN]	套接字
*@return	成功连接上为0，失败为错误码，见PC_Lib.h
*/
int  PCSockIsConnected(PC_SOCKET sock);

/**
*@brief		关闭一个已连接的SOCKET
*@param		sock	[IN]	套接字
*@return
*/
void PCShutdownSocket(PC_SOCKET sock);

/**
*@brief		创建一个Tcp套接字
*@param		nPort	[IN]	套接字需要绑定的端口，一般来说，只有监听套接字才需要指定端口号，其他传入-1即可
*			blocked	[IN]	是否阻塞，true阻塞 false不阻塞
*@return	成功则返回套接字描述符，失败返回PC_INVALID_SOCKET
*/
PC_SOCKET  PCCreateTcpSocket(int nPort, bool blocked);

/**
*@brief		解析一个IPv4地址，如果有多个则只会使用第一个，阻塞模式
*@param		pszHost		[IN]	一个点分十进制的IPv4地址或域名
*@param		nPort		[IN]	十进制端口号
*@param		addr4		[OUT]	输出解析后的地址
*@return	成功为0，失败为错误码，见PC_Lib.h
*/
int PCDnsParseAddrIPv4(const char *pszHost, int nPort, struct sockaddr_in *addr4);




//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
