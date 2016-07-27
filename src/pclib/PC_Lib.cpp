#include "PC_Lib.h"
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//调试类对象，唯一全局对象，保证此类最先初始化，最后反初始化。	//
//////////////////////////////////////////////////////////////////
CPCDebug g_PCDebug;

//pclib库调试日志根目录
#if defined (_WIN32)
    #define PC_LOG_BASE         "d:\\logs\\"
#else
    #define PC_LOG_BASE 		"/logs/"
#endif

PC_REC_MUTEX_HANDLE* CPCDebug::m_lock_cs = NULL;
bool CPCDebug::m_bUsedOnceAtLeast = false;
char CPCDebug::m_pszLogDir[PC_MAX_PATH] = {0};
CPCDebug::CPCDebug() 
{
	/* ---- 库初始化内容统一在此处，以下内容不允许调用本类和系统函数之外的函数 ---- */

	//设置字符编码
	setlocale(LC_ALL, "chs");

	//网络初始化
#if defined (_WIN32)
	//初始化winSocket环境
	WSADATA m_wsaData = { 0 };
	if (0 != WSAStartup(MAKEWORD(2, 2), &m_wsaData))
	{
		printf("WSAStartup ERROR！errno = %d.\n", WSAGetLastError());
		exit(1);
	}
	if (LOBYTE(m_wsaData.wVersion) != 2 || HIBYTE(m_wsaData.wVersion) != 2)
	{
		printf("WSAStartup return version(%04x) ERROR！\n", m_wsaData.wVersion);
		exit(1);
	}
#else
    //防止linux下网络通信时write函数产生的SIGPIPE信号导致程序退出
	signal(SIGPIPE, SIG_IGN);

    //注册中断键处理程序
    signal(SIGINT,CPCDebug::ExitPCLib);
#endif

	//OPENSSL初始化
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	//OPENSSL锁初始化
	m_lock_cs = (PC_REC_MUTEX_HANDLE*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(PC_REC_MUTEX_HANDLE));
	if (m_lock_cs == NULL)
	{
		printf("OPENSSL_malloc(%d) ERROR！\n", CRYPTO_num_locks());
		exit(1);
	}
	for (int i = 0; i < CRYPTO_num_locks(); i++)
	{
		PCInitRecMutex(&(m_lock_cs[i]));
	}

	//OPENSSL设置回调函数
	CRYPTO_THREADID_set_callback(PCSSL_ThreadID_CallBack);
	CRYPTO_set_locking_callback(PCSSL_Lock_CallBack);
	
    /* ----------------------------- 结束库初始化 ----------------------------- */

	//获取时间
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond;
	int nRet = PCGetSysTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);
	if (nRet != PC_RESULT_SUCCESS)
	{
		printf("Start ERROR！PCGetSysTime == %d\n", nRet);
		exit(1);
	}

	//获取进程名字
	char pszAppName[PC_MAX_PATH];
	if (PC_RESULT_SUCCESS != PCGetAppName(pszAppName))
	{
		strcpy(pszAppName, "__ERRAPPNAME__");
	}

	//库日志位置
	char pszLogRoot[PC_MAX_PATH];
    sprintf(pszLogRoot, "%spclib/",PC_LOG_BASE);
    PCMakeDir(PC_LOG_BASE);
    PCMakeDir(pszLogRoot);

	//第一级：/logs/pclib/2015_03_03/
	char pszTimeFolderName[PC_MAX_PATH] = { 0 };
	sprintf(pszTimeFolderName, "%s%4d_%02d_%02d/", pszLogRoot, nYear, nMonth, nDay);
	PCMakeDir(pszTimeFolderName);

	//第二级：/logs/pclib/2015_03_03/pszAppName_PID712/
    sprintf(m_pszLogDir, "%s%s_PID%lu/", pszTimeFolderName, pszAppName, static_cast<unsigned long>(PCGetCurrentProccessID()));
	PCMakeDir(m_pszLogDir);
}

CPCDebug::~CPCDebug()
{
	CPCDebug::CleanPCLib();
}

void CPCDebug::CleanPCLib()
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

	//如果从来没有写过日志，则删掉这个文件夹
	if (!m_bUsedOnceAtLeast)
	{
		rmdir(m_pszLogDir);
	}
}

void CPCDebug::ExitPCLib(int nExitCode)
{
	CPCDebug::CleanPCLib();
	exit(nExitCode);
	return;
}


void CPCDebug::PCInitRecMutex(PC_REC_MUTEX_HANDLE* mutex)
{
#if defined (_WIN32)
	InitializeCriticalSection(mutex);
#else
	//设置Linux下的mutex为递归的和范围为用于仅同步该进程中的线程 
	pthread_mutexattr_t locker_attr;
	int nErrorNo = pthread_mutexattr_init(&locker_attr);
	if(0 != nErrorNo)
	{
		printf( "pthread_mutexattr_init = %d！", nErrorNo);
        abort();
	}
    std::shared_ptr<pthread_mutexattr_t> pThreadMutexAttrPtr(&locker_attr, pthread_mutexattr_destroy);

	nErrorNo = pthread_mutexattr_settype(pThreadMutexAttrPtr.get(), PTHREAD_MUTEX_RECURSIVE);
	if(0 != nErrorNo)
	{
		printf("pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE) = %d！", nErrorNo);
        abort();
	}
	nErrorNo = pthread_mutexattr_setpshared(pThreadMutexAttrPtr.get(), PTHREAD_PROCESS_PRIVATE);
	if (0 != nErrorNo)
	{
		printf("pthread_mutexattr_setpshared(PTHREAD_PROCESS_PRIVATE) = %d！", nErrorNo);
        abort();
	}
	nErrorNo = pthread_mutex_init(mutex, pThreadMutexAttrPtr.get());
	PC_LOG_ASSERT(0 == nErrorNo, "pthread_mutex_init = %d！", nErrorNo);
	if (0 != nErrorNo)
	{
		printf("pthread_mutex_init = %d！", nErrorNo);
        abort();
	}
#endif
}
void CPCDebug::PCSSL_ThreadID_CallBack(CRYPTO_THREADID* id)
{
	CRYPTO_THREADID_set_numeric(id, PCGetCurrentThreadID());
}
void CPCDebug::PCSSL_Lock_CallBack(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		PC_REC_MUTEX_LOCK(&(m_lock_cs[type]));
	}
	else if (mode & CRYPTO_UNLOCK)
	{
		PC_REC_MUTEX_UNLOCK(&(m_lock_cs[type]));
	}
    PC_DEBUG_LOG("lock - mode=%d,type=%d,file=%s,line=%d",mode,type,file,line);
}

void CPCDebug::WriteLibLogFmt(const char* pFuncName, unsigned long ulLine, const char* pszLevel, const char* pszFmt, ...)
{
	//最后的错误码
	char szErrNo[25] = { 0 };
#if defined WIN32
	sprintf(szErrNo, "[ERRNO:%lu,%d]", GetLastError(), WSAGetLastError());
#else
	sprintf(szErrNo, "[ERRNO:%d]", errno);
#endif
	//参数检查
	if (pFuncName == NULL || pszFmt == NULL || pszLevel == NULL)
	{
		printf("CPCDebug::CPCDebug params err！\n");
		return;
	}

    //分配空间
    char pszFmtBuf[40960] = {0};

    //组织日志
    va_list ap;
    va_start(ap, pszFmt);
    int nRet = vsprintf(pszFmtBuf, pszFmt, ap);
    if(nRet < 0)
    {
        printf("CPCDebug::CPCDebug vsprintf error! maybe log is too long \n");
        return;
    }
    va_end(ap);

    //打开文件
    FILE  *pLogFile = NULL;
    char pszTimeBuf[PC_MAX_PATH] = { 0 };
    if (true == PCOpenFile(pLogFile, pszTimeBuf))
    {
        fprintf(pLogFile, "[%s][%s][%s][LINE%lu]%s%s\r\n", pszLevel, pszTimeBuf, pFuncName, ulLine, szErrNo, pszFmtBuf);
        fflush(pLogFile);
        fclose(pLogFile);
    }
    m_bUsedOnceAtLeast = true;

}


bool CPCDebug::PCOpenFile( FILE* &pLibLogFile, char pszTimeBuf[PC_MAX_PATH])
{
	if (pszTimeBuf == NULL)
	{
		printf("CPCDebug::PCOpenFile  ERROR！pszTimeBuf == NULL\n");
		return false;
	}

	//当前时间
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond;
	int nRet = PCGetSysTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);
	if (nRet != PC_RESULT_SUCCESS)
	{
		printf("CPCDebug::PCOpenFile  ERROR！PCGetSysTime == %d\n", nRet);
		return false;
	}
	
	if (pLibLogFile != NULL)
	{
		fflush(pLibLogFile);
		fclose(pLibLogFile);
		pLibLogFile = NULL;
	}

	//日志路径：/logs/pclib/2015_03_03/m_pszAppName_PID712/TID1265648745.log
	char pszLogFileName[PC_MAX_PATH] = { 0 };
	sprintf(pszLogFileName, "%sTID%lu.log", m_pszLogDir, PCGetCurrentThreadID());

	//打开文件
	pLibLogFile = fopen(pszLogFileName, "a+");
	if (NULL == pLibLogFile)
	{
		printf("CPCDebug::PCOpenFile  ERROR！fopen(%s) == NULL\n", pszLogFileName);
		return false;
	}
	sprintf(pszTimeBuf, "%4d_%02d_%02d_%02d:%02d:%02d_%03d", nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);
	return true;
}


//此处不可使用调试类打日志，因为调试类调用了此函数
int  CPCDebug::PCGetSysTime(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, int &nMilliSecond)
{
#if defined WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	nYear = st.wYear;
	nMonth = st.wMonth;
	nDay = st.wDay;
	nHour = st.wHour;
	nMinute = st.wMinute;
	nSecond = st.wSecond;
	nMilliSecond = st.wMilliseconds;
#else
	struct timeval  tvTime;
	if (-1 == gettimeofday(&tvTime, NULL))
	{
		return PC_RESULT_SYSERROR;
	}
	struct tm tmTime;
	if (NULL == localtime_r(&tvTime.tv_sec, &tmTime))
	{
		return PC_RESULT_SYSERROR;
	}
	nYear = 1900 + tmTime.tm_year;
	nMonth = 1 + tmTime.tm_mon;
	nDay = tmTime.tm_mday;
	nHour = tmTime.tm_hour;
	nMinute = tmTime.tm_min;
	nSecond = tmTime.tm_sec;
	nMilliSecond = tvTime.tv_usec / 1000;
#endif
	return PC_RESULT_SUCCESS;
}

//此处不可使用调试类打日志，因为调试类调用了此函数
int CPCDebug::PCGetAppName(char* szAppName)
{
	if (szAppName == NULL)
	{
		return PC_RESULT_PARAM;
	}
	char szAppPath[PC_MAX_PATH] = { 0 };
#if defined WIN32
	if (0 == GetModuleFileNameA(GetModuleHandleA(NULL), szAppPath, PC_MAX_PATH - 1))
#else
	if (-1 == readlink("/proc/self/exe", szAppPath, PC_MAX_PATH - 1))
#endif
	{
		return PC_RESULT_SYSERROR;
	}

	//从后到前找分隔符
	int nAppPathLen = static_cast<int>( strlen(szAppPath));
	for (int i = nAppPathLen - 1; i >= 0; i--)
	{
		if (szAppPath[i] == '\\' || szAppPath[i] == '/')
		{
			memcpy(szAppName, szAppPath + i + 1, nAppPathLen - i);
			szAppName[nAppPathLen - i] = '\0';
			return PC_RESULT_SUCCESS;
		}
	}

	return PC_RESULT_FORMATERROR;
}



//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
