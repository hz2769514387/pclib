#ifndef _PC_LOCK_H_
#define _PC_LOCK_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PC_Lib.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		可递归的锁。就是同一线程在获得锁后，再次加锁不会导致阻塞
*/
class CPCRecursiveLock :CPCNoCopyable
{
public:
	//nSpinCount旋转锁旋转的CPU周期数，仅windows下有效。参考值：0，4000
	explicit CPCRecursiveLock(unsigned long nSpinCount = 0);
	virtual ~CPCRecursiveLock();

	PC_REC_MUTEX_HANDLE&       operator*(void)       { return m_locker; }
	const PC_REC_MUTEX_HANDLE& operator*(void) const { return m_locker; }
public:
    void Lock(void) const;
    void UnLock(void) const;

private:
	//互斥量
    mutable PC_REC_MUTEX_HANDLE m_locker;
};

/**
*@brief		守卫类，用于线程共享数据保护。使用可递归的锁。
*/
class CPCGuard :CPCNoCopyable
{
public:
	explicit CPCGuard(CPCRecursiveLock& lock) : m_lock(lock)
	{
		m_lock.Lock();
	}
	~CPCGuard() throw ()
	{
		m_lock.UnLock();
	}
private:
	CPCRecursiveLock& m_lock;
};

/**
*@brief		条件变量类。
*/
class CPCCondition :CPCNoCopyable
{
public:
	explicit CPCCondition(CPCRecursiveLock& Mutex)
		: m_Mutex(Mutex)
	{
#if defined (_WIN32)
		InitializeConditionVariable(&m_Cond);
#else
		pthread_condattr_t cond_attr;
		int nErrorNo = pthread_condattr_init(&cond_attr);
		PC_ASSERT(0 == nErrorNo, "pthread_condattr_init = %d！", nErrorNo);
		std::shared_ptr<pthread_condattr_t> pThreadCondAttrPtr(&cond_attr, pthread_condattr_destroy);

		nErrorNo = pthread_condattr_setclock(pThreadCondAttrPtr.get(), CLOCK_MONOTONIC);
		PC_ASSERT(0 == nErrorNo, "pthread_condattr_setclock(CLOCK_MONOTONIC) = %d！", nErrorNo);

		nErrorNo = pthread_cond_init(&m_Cond, pThreadCondAttrPtr.get());
		PC_ASSERT(0 == nErrorNo, "pthread_cond_init = %d！", nErrorNo);
#endif
	}

	~CPCCondition(void)
	{
        NotifyAll();
		PC_CONDITION_DESTROY(&m_Cond);
	}

public:
	// 等待nTimeoutMs毫秒，nTimeoutMs如果<0代表永远等待。如果有信号则返回true
	bool Wait(int nTimeoutMs = -1) const
	{
#if defined (_WIN32)
		return !!SleepConditionVariableCS(&m_Cond, &(*m_Mutex), (nTimeoutMs < 0) ? INFINITE : nTimeoutMs);
#else
		int nErrorNo = -1;
		if (nTimeoutMs < 0)
		{
			nErrorNo = pthread_cond_wait(&m_Cond, &(*m_Mutex));
		}
		else
		{
			struct timespec ts;
            int nRet = clock_gettime(CLOCK_MONOTONIC, &ts);
            PC_ASSERT(nRet == 0,  "clock_gettime fail");

			ts.tv_sec += (nTimeoutMs/1000);
			ts.tv_nsec += (nTimeoutMs%1000*1000000);
			nErrorNo = pthread_cond_timedwait(&m_Cond, &(*m_Mutex), &ts);
		}
		return (nErrorNo == 0);
#endif
	}

	void Notify(void) const
	{
		PC_CONDITION_NOTIFY(&m_Cond);
	}

	void NotifyAll(void) const
	{
		PC_CONDITION_NOTIFYALL(&m_Cond);
	}

private:
    mutable PC_CONDITION_HANDLE m_Cond;
    CPCRecursiveLock&	m_Mutex;
};

/**
*@brief		JAVA中的同步计数器(CountDownLatch类)。
*/
class CPCCountDownLatch :CPCNoCopyable
{
public:
	explicit CPCCountDownLatch(int count)
		: m_Mutex()
		, m_Cond(m_Mutex)
		, m_Count(count)
	{
	}

	void Wait() const
	{
		CPCGuard lock(m_Mutex);
		while (m_Count > 0)
		{
			m_Cond.Wait();
		}
	}

	void CountDown()
	{
		CPCGuard lock(m_Mutex);
		--m_Count;
		if (m_Count == 0)
		{
			m_Cond.NotifyAll();
		}
	}

	int GetCount() const
	{
		CPCGuard lock(m_Mutex);
		return m_Count;
	}

private:
    mutable CPCRecursiveLock m_Mutex;
	CPCCondition	m_Cond;
	int				m_Count;
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif /*_PC_LOCK_H_*/
