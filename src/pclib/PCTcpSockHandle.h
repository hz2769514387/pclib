#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//每个事件投递的缓存数据的大小，此部分数据缓冲区需换入内核，要控制大小
#define MAX_IP_STR_LEN			(128)	//IP地址的点分十进制字符串最大长度


/**
*@brief		特定平台异步模型的结构定义
*/
#if defined (_WIN32)

//每次向IOCP投递请求的数据结构，这里主要是为了保存AcceptEx时的处理指针
class CPCTcpSockHandle;
typedef struct _IOCP_IO_CTX
{
	OVERLAPPED		m_olOriginal;	//原始重叠结构
	CPCTcpSockHandle*	m_pOwner;	//这个投递的数据所属于的连接
} IOCP_IO_CTX;

#else
#endif

/**
*@brief		TCP连接类，内部维护一个Socket
*			这个连接可以是服务端监听、处理、和客户端请求类型的连接
*			为了跨平台和避免内存泄漏，这个连接类将网络数据和操作绑定，即每个CPCTcpSockHandle对象
*				是有状态的，在特定的某个时刻只可能是一种状态，无法重叠操作（这种设计在windows平台上
*				有一定的性能损失）。
*/
class CPCTcpSockHandle : CPCNoCopyable
{
public:
	CPCTcpSockHandle();
	~CPCTcpSockHandle();

	//创建和清理连接函数。Create【主动创建服务器监听socket（nPort在（0~65535）之间，创建后自动开启监听）或客户端socket（nPort不合法）】 
	bool Create(int nPort, bool bBlock = false);
	void Cleanup();

	//投递请求，一般由用户主动调用<投递发送请求时，数据最大长度为 PER_SOCK_REQBUF_SIZE>
	bool PostConnect(const char *pszHostAddress, int nPort);
	bool PostSend(const char *szSendBuff, unsigned long nSendLen);
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//回调后的进一步处理，主要目的是为了屏蔽操作系统差异
	void ProcessAccept();
	void ProcessConnect();
	void ProcessSend(unsigned long dwSendedLen);
	void ProcessRecv(unsigned long dwRecvedLen);
	void ProcessClose();
	
	//完成请求后的进一步处理，子类实现这些函数处理业务逻辑
	virtual void OnAccepted(){}
	virtual void OnConnected(){}
	virtual void OnSendded(unsigned long dwSendedLen){}
	virtual void OnRecved(unsigned long dwRecvedLen){}
	virtual void OnClosed(){}

public:
	PC_SOCKET	m_hTcpSocket;					//内部维护的一个SOCKET
	bool		m_bListenSocket;				//SOCKET类型是否为监听类型
	char		m_pszRemoteIP[MAX_IP_STR_LEN];	//对方的IP地址(仅针对accept的socket)
	CPCRecursiveLock	m_Mutex;				//给子类提供的锁

	

#if defined (_WIN32)
	//操作码
	enum csEnumOpt
	{
		cseUnconnect = 0,
		cseConnect,
		cseAccept,
		cseRead,
		cseWrite
	};
	csEnumOpt		m_ConnOpt;						//操作码
	IOCP_IO_CTX		m_ioCtx;						//原始重叠结构
	char			m_szIOBuf[PER_SOCK_REQBUF_SIZE];//发送或接收的数据缓冲区
	unsigned long	m_dwIOBufLen;					//数据长度(发送时可指定长度，接收时此值就是接收数据的缓冲区长度)
	WSABUF			m_wsBufPointer;					//投递请求BUF指针
#else
    //对于Linux，内部需要维护epoll相关的的句柄
    int	m_epollFd;
    int m_eventFd;
#endif
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
