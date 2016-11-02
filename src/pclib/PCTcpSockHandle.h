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
*@brief		TCP连接类，内部维护一个Socket
*			这个连接可以是服务端监听、处理、和客户端请求类型的连接
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

	//完成请求后回调函数，子类实现即可，不需要用户主动调用(这里的实现仅用作测试，实际使用时需要编写子类覆盖这些方法)
	virtual void DoConnected(bool bSucceed){ PC_TRACE_LOG("connected"); }
	virtual void DoSendded(bool bSucceed, unsigned long dwSendedLen){ PC_TRACE_LOG("send(%lu) bytes", dwSendedLen); }
	virtual void DoRecved(bool bSucceed, const char *szRecvedBuff, unsigned long dwRecvedLen){ PC_TRACE_LOG("recv(%lu) bytes:%s", dwRecvedLen, szRecvedBuff); }
	virtual void DoClose(){ PC_TRACE_LOG("closed."); }

	//DoAccept提供了默认实现【被动接收socket，一般用于服务器接收到客户端的socket】
	virtual bool DoAccept(bool bSucceed, const char *szRecvedBuff, unsigned long dwRecvedLen);

public:
	PC_SOCKET	m_hTcpSocket;				//内部维护的一个SOCKET
	bool		m_bListenSocket;			//SOCKET类型是否为监听类型
	char	m_pszRemoteIP[MAX_IP_STR_LEN];	//对方的IP地址(仅针对accept的socket)
	CPCRecursiveLock	m_Mutex;			//给子类提供的锁

#if defined (_WIN32)
#else
    //对于Linux，内部需要维护epoll相关的的句柄
    int	m_epollFd;
    int m_eventFd;
#endif
};


/**
*@brief		特定平台异步模型的结构定义
*/
#if defined (_WIN32)

	//每次向IOCP投递请求的数据结构，因为可能要使用同时投递多次的功能，所以需要将所有待投递的数据如缓冲区等全部放入此结构中
	typedef struct _IOCP_IO_CTX
	{
		OVERLAPPED		m_olOriginal;	//原始重叠结构
		CPCTcpSockHandle*	m_pOwner;	//这个投递的数据所属于的连接
		BYTE			m_byOpType;		//操作类型

		char			m_szIOBuf[PER_SOCK_REQBUF_SIZE];//发送或接收的数据缓冲区
		unsigned long	m_dwIOBufLen;					//数据长度(发送时可指定长度，接收时此值就是接收数据的缓冲区长度)
		WSABUF			m_wsBufPointer;					//投递请求BUF指针

		// 初始化此结构
		_IOCP_IO_CTX(BYTE byOpType, CPCTcpSockHandle* pOwner, const char* szIOBuf = NULL, unsigned long dwIOBufLen = PER_SOCK_REQBUF_SIZE)
		{
			ZeroMemory(&m_olOriginal, sizeof(m_olOriginal));
			m_pOwner = pOwner;
			m_byOpType = byOpType;
			ZeroMemory(m_szIOBuf, sizeof(m_szIOBuf));
			m_dwIOBufLen = sizeof(m_szIOBuf);
			if (szIOBuf)
			{
				if (dwIOBufLen <= PER_SOCK_REQBUF_SIZE)
				{
					memcpy(m_szIOBuf, szIOBuf, dwIOBufLen);
					m_dwIOBufLen = dwIOBufLen;
				}
				else
				{
					PC_WARN_LOG("dwIOBufLen(%ld) > %d, only copy(%d) length.", dwIOBufLen, PER_SOCK_REQBUF_SIZE, PER_SOCK_REQBUF_SIZE);
					memcpy(m_szIOBuf, szIOBuf, PER_SOCK_REQBUF_SIZE);
					m_dwIOBufLen = PER_SOCK_REQBUF_SIZE;
				}
			}
			m_wsBufPointer.buf = m_szIOBuf;
			m_wsBufPointer.len = m_dwIOBufLen;
		}

	} IOCP_IO_CTX;

	//操作类型，对应_IOCP_IO_CTX.m_byOpType
	#define OP_UNKNOWN		0xFF
	#define OP_READ			0
	#define OP_WRITE		1
	#define OP_CONNECT		2
	#define OP_ACCEPT		3
#else

#endif

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
