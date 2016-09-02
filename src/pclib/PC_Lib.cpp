#include "PC_Lib.h"
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

PC_REC_MUTEX_HANDLE* CPCLib::m_lock_cs = NULL;
CPCLib::CPCLib() 
{
	/* ---- 库初始化内容统一在此处，以下内容不允许调用本类和系统函数之外的函数 ---- */

	//设置字符编码
#if defined (_WIN32)
	PC_ASSERT(setlocale(LC_ALL, "chs"), "INIT setlocale(LC_ALL, chs) fail!");
#else
	PC_ASSERT(setlocale(LC_ALL, "zh_CN.gbk"), "INIT setlocale(LC_ALL, zh_CN.gbk) fail!");
#endif

	//网络初始化
#if defined (_WIN32)
	//初始化winSocket环境
	WSADATA m_wsaData = { 0 };
	PC_ASSERT(0 == WSAStartup(MAKEWORD(2, 2), &m_wsaData), "WSAStartup ERROR！errno = %d.\n", WSAGetLastError());
	PC_ASSERT(LOBYTE(m_wsaData.wVersion) == 2, "WSAStartup return version(%04x) ERROR！\n", m_wsaData.wVersion);
	PC_ASSERT(HIBYTE(m_wsaData.wVersion) == 2, "WSAStartup return version(%04x) ERROR！\n", m_wsaData.wVersion);
	
#else
    //防止linux下网络通信时write函数产生的SIGPIPE信号导致程序退出
	signal(SIGPIPE, SIG_IGN);
#endif

	//OPENSSL初始化
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	//OPENSSL锁初始化
	m_lock_cs = (PC_REC_MUTEX_HANDLE*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(PC_REC_MUTEX_HANDLE));
	PC_ASSERT(m_lock_cs != NULL, "OPENSSL_malloc(%d) ERROR！\n", CRYPTO_num_locks());
	
	for (int i = 0; i < CRYPTO_num_locks(); i++)
	{
		PCInitRecMutex(&(m_lock_cs[i]));
	}

	//OPENSSL设置回调函数
	CRYPTO_THREADID_set_callback(PCSSL_ThreadID_CallBack);
	CRYPTO_set_locking_callback(PCSSL_Lock_CallBack);
	
    /* ----------------------------- 结束库初始化 ----------------------------- */
}

CPCLib::~CPCLib()
{
	//清理WSA库
#if defined (_WIN32)
	WSACleanup();
#endif

	//OPENSSL释放
	CRYPTO_THREADID_set_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	if (m_lock_cs)
	{
		for (int i = 0; i<CRYPTO_num_locks(); i++)
		{
			PC_REC_MUTEX_DESTROY(&(m_lock_cs[i]));
		}
		OPENSSL_free(m_lock_cs);
		m_lock_cs = NULL;
	}
}


void CPCLib::PCInitRecMutex(PC_REC_MUTEX_HANDLE* mutex)
{
#if defined (_WIN32)
	InitializeCriticalSection(mutex);
#else
	//设置Linux下的mutex为递归的和范围为用于仅同步该进程中的线程 
	pthread_mutexattr_t locker_attr;
	int nErrorNo = pthread_mutexattr_init(&locker_attr);
	PC_ASSERT(0 == nErrorNo, "pthread_mutexattr_init = %d！", nErrorNo);
	
    std::shared_ptr<pthread_mutexattr_t> pThreadMutexAttrPtr(&locker_attr, pthread_mutexattr_destroy);

	nErrorNo = pthread_mutexattr_settype(pThreadMutexAttrPtr.get(), PTHREAD_MUTEX_RECURSIVE);
	PC_ASSERT(0 == nErrorNo, "pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE) = %d！", nErrorNo);
	
	nErrorNo = pthread_mutexattr_setpshared(pThreadMutexAttrPtr.get(), PTHREAD_PROCESS_PRIVATE);
	PC_ASSERT(0 == nErrorNo, "pthread_mutexattr_setpshared(PTHREAD_PROCESS_PRIVATE) = %d！", nErrorNo);
	
	nErrorNo = pthread_mutex_init(mutex, pThreadMutexAttrPtr.get());
	PC_ASSERT(0 == nErrorNo, "pthread_mutex_init = %d！", nErrorNo);
	
#endif
}
void CPCLib::PCSSL_ThreadID_CallBack(CRYPTO_THREADID* id)
{
	CRYPTO_THREADID_set_numeric(id, PCGetCurrentThreadID());
}
void CPCLib::PCSSL_Lock_CallBack(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		PC_REC_MUTEX_LOCK(&(m_lock_cs[type]));
	}
	else if (mode & CRYPTO_UNLOCK)
	{
		PC_REC_MUTEX_UNLOCK(&(m_lock_cs[type]));
	}
	PC_TRACE_LOG("lock - mode=%d,type=%d,file=%s,line=%d", mode, type, file, line);
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
