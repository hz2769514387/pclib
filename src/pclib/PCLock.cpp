#include "PC_Lib.h"
#include "PCMemory.h"
#include "PCLock.h" \

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

CPCRecursiveLock::CPCRecursiveLock(unsigned long nSpinCount)
{
#if defined (_WIN32)
	//Windows下的临界区默认为递归的，使用旋转锁在临界区执行时间较短的情况下可优化性能（如果临界区执行时间较长，那在指定的旋转数目个CPU周期内可能造成CPU BUSY飙高）。
	BOOL bRet = InitializeCriticalSectionAndSpinCount(&m_locker, nSpinCount);
	PC_LOG_ASSERT(bRet, "InitializeCriticalSectionAndSpinCount(nSpinCount=%d) = FALSE！", nSpinCount);
#else
	//设置Linux下的mutex为递归的和范围为用于仅同步该进程中的线程
	pthread_mutexattr_t lock_attr;
	int nErrorNo = pthread_mutexattr_init(&lock_attr);
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutexattr_init = %d！", nErrorNo);
	std::shared_ptr<pthread_mutexattr_t> pThreadMutexAttrPtr(&lock_attr, pthread_mutexattr_destroy);

	nErrorNo = pthread_mutexattr_settype(pThreadMutexAttrPtr.get(), PTHREAD_MUTEX_RECURSIVE);
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE) = %d！", nErrorNo);

	nErrorNo = pthread_mutexattr_setpshared(pThreadMutexAttrPtr.get(), PTHREAD_PROCESS_PRIVATE);
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutexattr_setpshared(PTHREAD_PROCESS_PRIVATE) = %d！", nErrorNo);

	nErrorNo = pthread_mutex_init(&m_locker, pThreadMutexAttrPtr.get());
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutex_init = %d！", nErrorNo);

#endif
}

CPCRecursiveLock::~CPCRecursiveLock()
{
	PC_REC_MUTEX_DESTROY(&m_locker);
}

void CPCRecursiveLock::Lock(void)  const
{
#if defined (_WIN32)
	EnterCriticalSection(&m_locker);
#else
	int nErrorNo = pthread_mutex_lock(&m_locker);
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutex_lock = %d！", nErrorNo);
#endif
}

void CPCRecursiveLock::UnLock(void)  const
{ 
#if defined (_WIN32)
	LeaveCriticalSection(&m_locker);
#else
	int nErrorNo = pthread_mutex_unlock(&m_locker);
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutex_unlock = %d！", nErrorNo);
#endif
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////


