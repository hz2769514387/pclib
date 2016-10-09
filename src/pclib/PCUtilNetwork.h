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
*@brief		创建一个指定客户端或服务端类型的Tcp套接字
*@param		service	[IN]	当service为NULL或""则此套接字为客户端套接字
*							当service为十进制端口号或服务名(http,ftp..etc)此套接字为服务端套接字，函数会执行监听动作
*			blocked	[IN]	是否阻塞，true阻塞 false不阻塞
*@return	成功则返回套接字描述符，失败返回PC_INVALID_SOCKET
*/
PC_SOCKET  PCCreateTcpSocket(const char * service, bool blocked);

/**
*@brief		解析一个IPv4地址，如果有多个则只会使用第一个，阻塞模式
*@param		hostname4	[IN]	一个点分十进制的IPv4地址或域名
*@param		service		[IN]	十进制端口号或服务名(http,ftp..etc)
*@param		addr4		[OUT]	输出解析后的地址
*@return	成功为0，失败为错误码，见PC_Lib.h
*/
int PCDnsParseAddrIPv4(const char *hostname4, const char * service, struct sockaddr_in *addr4);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
