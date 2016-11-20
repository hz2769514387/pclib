#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"
#include "PCBuffer.h"
#include "PCTcpPoller.h" 
#include "PCUtilMisc_Linux.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//每个事件投递的缓存数据的大小
#define MAX_IP_LEN				(128)	//IP地址的点分十进制字符串最大长度

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
	//连接类型
	enum eSockType
	{
		eListenType = 0,	//监听类型（一般用于服务端监听套接字）
		eAcceptType,		//接收连接类型（一般用于服务端处理套接字）
		eConnectType		//主动连接类型（一般用于客户端）
	};
	CPCTcpSockHandle(eSockType eType);
	~CPCTcpSockHandle();

	//////////////////////////////////////////////////////////////////////////////
	// 接口函数，由用户主动调用
	//<创建和清理函数为同步处理，不会触发虚函数通知；投递函数则可能会触发通知>
	//////////////////////////////////////////////////////////////////////////////
	
	//创建函数：【主动创建服务器监听socket（nPort在[0~65535]之间，创建后自动开启监听）或客户端socket（nPort不合法）】 
	bool Create(int nPort, bool bBlock = false);
	//清理函数：bGracefully = true 时，优雅地关闭连接，否则为强制关闭连接。
	void Cleanup(bool bGracefully = false);
	
	//投递请求，一般由用户主动调用，投递发送请求之前，需要自行将数据放入m_SendBuffer
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
    virtual void OnAccepted(){}     //仅服务端处理套接字 需要实现此接口
    virtual void OnConnected(){}    //仅客户端 需要实现此接口
	virtual void OnSendded(){}
	virtual void OnRecved(unsigned long dwRecvedLen){}
	virtual void OnClosed(){}

public:
	CPCRecursiveLock	m_Mutex;			//给子类提供的锁
	eSockType	m_SocketType;				//SOCKET类型
	PC_SOCKET	m_SocketFd;					//内部维护的一个SOCKET
	PC_SOCKET	m_ListenSocketFd;			//对于Accept类型的socket，内部维护监听socket的描述符
	char		m_pszRemoteIP[MAX_IP_LEN];	//对方的IP地址(仅针对accept的socket)
	CPCBuffer	m_SendBuffer;				//单次发送缓冲区
	CPCBuffer	m_RecvBuffer;				//单次接收缓冲区

#if defined (_WIN32)
	//操作码
	enum eOpt
	{
		eUnconnect = 0,
		eConnect,
		eAccept,
		eRead,
		eWrite
	};
	eOpt			m_Opt;					//操作码
	IOCP_IO_CTX		m_ioCtx;				//原始重叠结构
	WSABUF			m_wsBufPointer;			//投递请求BUF指针
#else
    //对于Linux，内部需要维护CPCTcpPollerThread指针
	CPCTcpPollerThread* m_pPollerThread ;	//epoll_wait线程
	int				m_events;				//要提交的事件，可以是以下组合：EPOLLIN EPOLLOUT EPOLLPRI EPOLLERR EPOLLHUP EPOLLET EPOLLONESHOT
	int				m_epctlOp;				//要对事件进行的动作，可以是：EPOLL_CTL_ADD EPOLL_CTL_MOD EPOLL_CTL_DEL

    //Epoll注册事件
    bool EpollEventCtl(int events, int epctlOp)
    {
        m_events = events;
        m_epctlOp = epctlOp;

        /************************************************************
        *epoll_ctl请求处理,此处跨线程调用了，不安全。最好使用发线程消息的方式
        ************************************************************/
        //先将事件删除
        if(epctlOp == EPOLL_CTL_ADD)
        {
            LIN_EpollEventCtl(m_pPollerThread->m_epollFd, m_SocketFd,  EPOLL_CTL_DEL, EPOLLIN | EPOLLOUT |  EPOLLERR | EPOLLHUP | EPOLLET, this);
        }

        //在执行事件操作
        return (0 == LIN_EpollEventCtl(m_pPollerThread->m_epollFd, m_SocketFd,  m_epctlOp, m_events, this));
    }
#endif

private:
	size_t	m_ActualSendedLen;		//实际发送的长度
};


//限制模板类CPCListenManager的模板参数必须继承于CPCTcpSockHandle(现在只能运行期限制，完美的解决方案应该是编译期限制)
template <typename T, bool nouse=std::is_base_of<CPCTcpSockHandle, T>::value>
struct CPCListenManager{CPCListenManager(){PC_ASSERT(false,"This Should't compile.init listen manager type must extend CPCTcpSockHandle");}};

/**
*@brief		TCP监听连接管理类
*			此为模板类，传入的模板参数类型T是eAcceptType的处理连接类，必须限制是CPCTcpSockHandle的子类，否则会运行崩溃
*           要实现一个tcp server，直接使用此类作为监听对象，实现具体的处理连接类，然后启动即可。
*/
template <typename T>
class CPCListenManager<T,true>  : public CPCTcpSockHandle
								, public CPCThread
{
public:
   CPCListenManager()
       :CPCTcpSockHandle(eSockType::eListenType)
       ,m_Started(false)
       ,m_Stoped(true)
   {

   }
   ~CPCListenManager()
   {
	   StopListen();
   }

   bool StartListen(int nPort, unsigned int nHandleCount)
   {
        if(m_Started)
        {
            PC_TRACE_LOG("listen(port=%d) already started!", nPort);
            return true;
        }

		//参数校验
		if (nPort < 0 || nPort > 65535 || nHandleCount == 0)
		{
			PC_ERROR_LOG("parms err!nPort=%d, nHandleCount=%u. ", nPort, nHandleCount);
			return false;
		}

		//开始监听
		if (false == this->Create(nPort))
		{
			return false;
		}

		//创建accept处理对象
		m_AcceptHandleList.clear();
		for (unsigned int i = 0; i < nHandleCount; i++)
		{
			T* tmpAcceptHandle = new (std::nothrow) T();
			if (tmpAcceptHandle == NULL)
			{
				PC_WARN_LOG("listen(%d) warning! new accept handle fail! this handle[%u] is invalid.", nPort, i);
				continue;
			}
			if (false == tmpAcceptHandle->PostAccept(this->m_SocketFd))
			{
				PC_WARN_LOG("listen(%d) warning! post accept fail! this handle[%u] is invalid.", nPort, i);
				delete tmpAcceptHandle;
				tmpAcceptHandle = NULL;
				continue;
			}
			m_AcceptHandleList.push_back(tmpAcceptHandle);
		}

		//启动管理线程
		if (false == this->StartThread(PC_THREAD_TIMEOUT_MS))
		{
			this->Cleanup();
			return false;
		}
        m_Started = true;
        return true;
   }

   void StopListen()
   {
        if(m_Stoped)
        {
            PC_TRACE_LOG("listen  already stoped!");
            return;
        }
		this->StopThread(PC_THREAD_TIMEOUT_MS);
		this->Cleanup();
        m_Stoped = true;
   }

   void Svc()
   {
	   while (m_bRunning)
	   {

	   }
   }

private:
   std::vector<T*>  m_AcceptHandleList;
   bool             m_Started;
   bool             m_Stoped;
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
