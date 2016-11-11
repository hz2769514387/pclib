#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"
#include "PCBuffer.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//每个事件投递的缓存数据的大小
#define MAX_IP_LEN				(128)	//IP地址的点分十进制字符串最大长度


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
*@brief		TCP连接类，内部维护一个Socket。这个连接可以是服务端监听、处理、和客户端请求类型的连接
*			为了跨平台和避免内存泄漏，这个连接类将网络数据和操作绑定，即每个CPCTcpSockHandle对象
*				是有状态的，在特定的某个时刻只可能是一种状态，无法重叠操作（这种设计在windows平台上有一定的性能损失）。
*			网络架构设计为三层：自底向上依次为：
*				1.网络事件分发层，是最底层的网络事件源。由CPCTcpPoller和CPCTcpPollerThread共同完成；不管理buffer，用户只需要启动和关闭它，其他无需过问
*				2.网络事件处理层，完成除超时外的大部分网络事件的处理，同时抹平操作系统差异。由CPCTcpSockHandle完成；分别管理发送buffer和接收buffer
*				3.网络数据处理层，处理上层业务逻辑。由用户自己继承CPCTcpSockHandle类并实现其虚函数完成；用户可以完全利用第二层的发送buffer（即可以将需要
*					投递的数据打包到第二层的发送buffer中，然后投递发送请求）；不能完全依赖第二层的接收buffer（分多次接收时需要自己保存每次接收的结果）
*/
class CPCTcpSockHandle : CPCNoCopyable
{
public:
	CPCTcpSockHandle();
	~CPCTcpSockHandle();

	//////////////////////////////////////////////////////////////////////////////
	// 接口函数，由用户主动调用
	//////////////////////////////////////////////////////////////////////////////
	//创建函数：【主动创建服务器监听socket（nPort在（0~65535）之间，创建后自动开启监听）或客户端socket（nPort不合法）】 
	bool Create(int nPort, bool bBlock = false);
	//清理函数：bGracefully = true 时，优雅地关闭连接，否则为强制关闭连接。
	void Cleanup(bool bGracefully = false);
	//投递请求，一般由用户主动调用<投递发送请求时，数据最大长度为 PER_SOCK_REQBUF_SIZE>
	bool PostConnect(const char *pszHostAddress, int nPort);
	bool PostSend();
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//////////////////////////////////////////////////////////////////////////////
	// 已经实现的回调函数，回调后的进一步处理，主要目的是为了屏蔽操作系统差异，用户不要调用
	//////////////////////////////////////////////////////////////////////////////
	void ProcessAccept();
	void ProcessConnect();
	void ProcessSend(unsigned long dwSendedLen);
	void ProcessRecv(unsigned long dwRecvedLen);
	void ProcessClose();
	
	//////////////////////////////////////////////////////////////////////////////
	// 未实现的回调函数，完成请求后的进一步处理，由用户继承此类后实现这些函数处理业务逻辑
	//////////////////////////////////////////////////////////////////////////////
	virtual void OnAccepted(){}
	virtual void OnConnected(){}
	virtual void OnSendded(){}
	virtual void OnRecved(unsigned long dwRecvedLen){}
	virtual void OnClosed(){}

public:
	CPCRecursiveLock	m_Mutex;			//给子类提供的锁
	PC_SOCKET	m_hTcpSocket;				//内部维护的一个SOCKET
	bool		m_bListenSocket;			//SOCKET类型是否为监听类型
	char		m_pszRemoteIP[MAX_IP_LEN];	//对方的IP地址(仅针对accept的socket)
	CPCBuffer	m_SendBuffer;				//单次发送缓冲区
	CPCBuffer	m_RecvBuffer;				//单次接收缓冲区

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
	csEnumOpt		m_ConnOpt;				//操作码
	IOCP_IO_CTX		m_ioCtx;				//原始重叠结构
	WSABUF			m_wsBufPointer;			//投递请求BUF指针
#else
    //对于Linux，内部需要维护epoll相关的的句柄
    int	m_epollFd;
    int m_eventFd;
#endif

private:
	unsigned int	m_ActualSendedLen;		//实际发送的长度
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
