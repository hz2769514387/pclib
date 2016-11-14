#include "PC_Lib.h"
#include "PCLock.h" 
#include "PCTimeValue.h" 
#include "PCThread.h"  
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


CPCThread::CPCThread()
	: m_bIsCreated(false)
	, m_bRunning(false)
    , m_bThreadExited(false)
	, m_nStackSize(0)
    , m_threadId(0)
{
}

void CPCThread::SetStackSize(unsigned int nStackSize)
{
	if (nStackSize < PC_MIN_THREAD_STACK_SIZE)
	{
		m_nStackSize = 0;
	}
	else
	{
		m_nStackSize = nStackSize;
	}
}

//线程入口点
#ifdef  __cplusplus
extern "C" {
#endif

	#if defined (_WIN32)
	unsigned int __stdcall	PCThreadStartAddr(void * obj)
	#else
	void*	PCThreadStartAddr(void * obj)
	#endif
    {
		CPCThread* pThrBase = static_cast<CPCThread*>(obj);
		PC_ASSERT(pThrBase != NULL, "static_cast<CPCThreadBase*>(obj) = NULL！");

		PC_TRACE_LOG("THREAD PCThreadStartAddr START!");
        pThrBase->SetThreadId( static_cast<int> ( PCGetCurrentThreadID() ) );
		pThrBase->SetRunning(true);
		pThrBase->SetExited(false);
		pThrBase->Svc();
		pThrBase->SetRunning(false);
		pThrBase->SetExited(true);
        PC_TRACE_LOG("THREAD PCThreadStartAddr END!");
		
	#if defined (_WIN32)
        return 0;
	#else
        return ((void*)0);
	#endif
	}

#ifdef  __cplusplus
}
#endif

bool CPCThread::StartThread(int nTimeoutMs)
{
	if (m_bIsCreated)
	{
		PC_ERROR_LOG("ERROR! - THREAD is Already Created");
		return false;
	}
	CPCTimeValue tvStart = CPCTimeValue::Now();

	//创建线程
#if defined (_WIN32)
    unsigned int   uThreadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, m_nStackSize, PCThreadStartAddr, this, 0, &uThreadId);
	if (hThread == NULL)
	{
		PC_ERROR_LOG("_beginthreadex fail! m_nStackSize=%u", m_nStackSize);
		return false;
	}
	CloseHandle(hThread);
#else
	//设置线程属性为可分离的
	pthread_attr_t pthread_attr;
	int nErrorNo = pthread_attr_init(&pthread_attr);
	PC_ASSERT(0 == nErrorNo, "pthread_attr_init = %d！", nErrorNo);
    std::shared_ptr<pthread_attr_t> pThreadAttrPtr(&pthread_attr, pthread_attr_destroy);

	nErrorNo = pthread_attr_setdetachstate(pThreadAttrPtr.get(), PTHREAD_CREATE_DETACHED);
	if (0 != nErrorNo)
	{
		PC_ERROR_LOG("pthread_attr_setdetachstate = %d fail!",nErrorNo);
		return false;
	}
	//设置线程栈大小
	if(m_nStackSize > 0)
	{
		nErrorNo = pthread_attr_setstacksize(pThreadAttrPtr.get(), m_nStackSize);
		if (0 != nErrorNo)
		{
			PC_ERROR_LOG("pthread_attr_setstacksize(%u) = %d fail!", m_nStackSize, nErrorNo);
			return false;
		}
    }
	//创建线程
    pthread_t tThreadHandle;
    nErrorNo = pthread_create(&tThreadHandle, pThreadAttrPtr.get(), PCThreadStartAddr, this);
	if (0 != nErrorNo)
	{
		PC_ERROR_LOG("pthread_create = %d fail! m_nStackSize=%u", nErrorNo, m_nStackSize);
		return false;
    }
#endif
	
	while (!m_bRunning)
	{
        if (tvStart.IsTimeOut(nTimeoutMs))
		{
			PC_ERROR_LOG("thread start timeout!( %d ms)", nTimeoutMs);
			return false;
		}
		PCSleepMsec(5);
	}
	m_bIsCreated = true;
	PC_TRACE_LOG("THREAD Created SUCCESS! m_nStackSize=%u", m_nStackSize);
	return true;
}

void	CPCThread::StopThread(int nTimeoutMs)
{
	if (!m_bIsCreated || m_bThreadExited)
	{
		return;
	}
	CPCTimeValue tvStart = CPCTimeValue::Now();

	m_bRunning = false;
	while (!m_bThreadExited)
	{
        if (tvStart.IsTimeOut(nTimeoutMs))
		{
            PC_ERROR_LOG("thread(%d) stop timeout!( %d ms)", m_threadId, nTimeoutMs);
			return;
		}
		PCSleepMsec(5);
	}
	m_bIsCreated = false;
}	


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
