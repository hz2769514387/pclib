#ifndef _PC_TCPCONNECTION_H_
#define _PC_TCPCONNECTION_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define MAX_SOCK_CACHE_SIZE		(4096)	//连接类缓存数据的大小，此部分数据缓冲区需换入内核，要控制大小
#define MAX_IP_STR_LEN			(128)	//IP地址的点分十进制字符串最大长度

/**
*@brief		特定异步模型的结构定义
*/
#if defined (_WIN32)

	//重叠结构EX
	typedef struct _OVERLAPPEDPLUS
	{
		OVERLAPPED				ol;			//原始重叠结构
		BYTE					byOpCode;	//操作码
		class CTcpConnection	*hClient;	//这个指针保存了投递accept的处理socket的指针
	} OVERLAPPEDPLUS;

	//操作码，对应重叠结构byOpCode
	#define OP_READ			0
	#define OP_WRITE		1
	#define OP_CONNECT		2
	#define OP_ACCEPT		3
#else

#endif

/**
*@brief		TCP连接类，内部维护一个Socket
*			这个连接可以是服务端监听、处理、和客户端请求类型的连接
*/
class CTcpConnection : CPCNoCopyable
{
public:
	CTcpConnection();
	~CTcpConnection();

	//创建和清理连接函数。Create【主动创建服务器监听socket（service不为空，创建后自动开启监听）或客户端socket（service为空）】 
	bool Create(const char * service, bool bBlock = false);
	void Cleanup();

	//投递请求，一般由用户主动调用
	bool PostConnect(const char *pszHostAddress, const char * service);
	bool PostSend();
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//完成请求后回调函数，子类实现即可，不需要用户主动调用
	virtual void DoConnected(){}
	virtual void DoSendded(unsigned long dwSize){}
	virtual void DoRecved(unsigned long dwSize){}
	virtual void DoClose(){}

	//DoAccept提供了默认实现【被动接收socket，一般用于服务器接收到客户端的socket】
	virtual bool DoAccept(unsigned long dwSize);

public:
	char m_szSendBuff[MAX_SOCK_CACHE_SIZE];	//待发送的数据缓冲区
	unsigned long m_dwSendLen;				//待发送的数据长度
	char m_szRecvBuff[MAX_SOCK_CACHE_SIZE];	//接收缓冲区

	PC_SOCKET	m_hTcpSocket;				//内部维护的一个SOCKET
	bool		m_bListenSocket;			//SOCKET类型是否为监听类型
	char	m_pszRemoteIP[MAX_IP_STR_LEN];	//对方的IP地址

protected:
	OVERLAPPEDPLUS		m_overLapped;		//接收和连接的重叠结构
	OVERLAPPEDPLUS		m_sendOverLapped;	//发送的重叠结构	
	WSABUF				m_wsBuff;			//接收和连接的BUF指针
	WSABUF				m_wsSendBuff;		//发送的BUF指针
	CPCRecursiveLock	m_Mutex;			//给子类提供的锁
};



//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPCONNECTION_H_*/
