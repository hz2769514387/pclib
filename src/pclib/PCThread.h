#ifndef _PCTHREADBASE_H__
#define _PCTHREADBASE_H__
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCMemory.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//线程栈大小限制
#define PC_MIN_THREAD_STACK_SIZE	(16384)

//线程退出时的默认超时时间(毫秒)
#define PC_THREAD_TIMEOUT_MS        (5000)

/**
*@brief		线程类
*			
*/
class CPCThread :CPCNoCopyable
{
public:
	explicit CPCThread();
    virtual ~CPCThread(){ StopThread(PC_THREAD_TIMEOUT_MS); }
	
	//线程的创建启动和停止，只有线程完全启动或完全停止才返回。你可以设置等待多少毫秒后超时返回(<0代表一直等待)
	bool	StartThread(int nTimeoutMs = -1);	
    void	StopThread(int nTimeoutMs = -1);

    //线程的入口，需要覆盖此函数；
    virtual void Svc() = 0;

	//不常用函数
	void SetStackSize(unsigned int nStackSize);						//设置线程栈大小，必须在StartThread之前调用才有效，用户一般无需关心
	void SetRunning(bool bRunning)	{ m_bRunning = bRunning; }		//m_bRunning设置，用户一般无需关心
	void SetExited(bool bExited)	{ m_bThreadExited = bExited; }	//m_bThreadExited设置，用户一般无需关心
    int  GetThreadId() const        { return m_threadId; }          //获取线程ID，只有线程创建成功并且运行起来后才可以调用
	void SetThreadId(int threadId)  { m_threadId = threadId; }      //不要调用这个函数！
protected:
	
	unsigned int		m_nStackSize;		//线程栈大小
    int                 m_threadId;         //线程ID
	CPCRecursiveLock	m_Mutex;			//给子类提供的锁

	bool				m_bRunning;			//线程是否正在运行，此变量一般用作循环变量和主线程判断子线程是否启动
	bool				m_bThreadExited;	//线程是否已经退出了，一般用于主线程等待子线程退出
	bool				m_bIsCreated;		//线程是否已经创建
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif/*_THREADBASE_H__*/
