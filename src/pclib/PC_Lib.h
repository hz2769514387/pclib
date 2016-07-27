#ifndef _PCLIB__H_
#define _PCLIB__H_
#if defined(_MSC_VER)
# pragma once
#endif

/*----------------------------�ӿ�ͳһ������-----------------------------------*/
#define PC_RESULT_SUCCESS			( 0)	//�ɹ�
#define	PC_RESULT_PARAM				(-1)	//��������
#define	PC_RESULT_SYSERROR			(-2)	//ϵͳ�쳣
#define	PC_RESULT_BUFFNOT_ENOUGH	(-3)	//����Ŀռ䳤�Ȳ���
#define	PC_RESULT_MEMORY			(-4)	//�����ڴ�ʧ��
#define	PC_RESULT_FORMATERROR		(-5)	//�ַ�����ʽ����
#define	PC_RESULT_HANDLEINVALID		(-6)	//�����õľ��
#define	PC_RESULT_FILEOPEN			(-7)	//���ļ�ʧ��
#define	PC_RESULT_FILEREAD			(-8)	//���ļ�����
#define	PC_RESULT_TIMEOUT			(-9)	//������ʱ
#define	PC_RESULT_NOMOREDATA		(-10)	//û�и���������
#define	PC_RESULT_SERVICE_NOTEXIST	(-11)	//ָ���ķ���δ��װ��windows��

/*----------------------------ͨ�ú궨��---------------------------------------*/
//PCLib��汾������PCLib�Ȿ�������е���������ַ���Ĭ�϶�ΪGBK���룬����������˵����
#define PC_LIB_VERSION			"V1.0.0.0"

//���ֿռ�
#define PCLIB_NAMESPACE_BEG		namespace pclib {
#define PCLIB_NAMESPACE_END		}

//����·������
#define PC_MAX_PATH				(260)

//URL·������
#define PC_MAX_URL				(2084)


/*----------------------------��ʵ�ֵĲ��ֿ�ƽ̨����---------------------------*/
//��ȡ��ǰ�߳�ID������ULONG,LINUX����int
#if defined (_WIN32)
	#define PCGetCurrentThreadID()	GetCurrentThreadId()
#else
	#define PCGetCurrentThreadID()	syscall(SYS_gettid)
#endif

//��ȡ��ǰ����ID��WINDOWS����ULONG,LINUX����int
#if defined (_WIN32)
	#define PCGetCurrentProccessID	GetCurrentProcessId
#else
	#define PCGetCurrentProccessID	getpid
#endif

//�����ִ�Сд���ַ����Ƚ�
#if defined (_WIN32)
	#define PCStrCaseCmp	_stricmp
#else
	#define PCStrCaseCmp	strcasecmp
#endif

//�����ִ�Сд���ַ���ָ�����ȱȽ�
#if defined (_WIN32)
	#define PCStrnCaseCmp	_strnicmp
#else
	#define PCStrnCaseCmp	strncasecmp
#endif

//��ȡ��ǰ�Ĺ���Ŀ¼
#if defined (_WIN32)
	#define PCGetCwd	_getcwd
#else
	#define PCGetCwd	getcwd
#endif

//�����ļ��У�����ֵ���ɹ�0 ʧ��-1
#if defined (_WIN32)
	#define PCMakeDir(_DIRNAME)	_mkdir(_DIRNAME)
#else
	#define PCMakeDir(_DIRNAME)	mkdir(_DIRNAME, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

//����ָ��������
#if defined (_WIN32)
	#define PCSleepMsec(_MSEC)	Sleep(_MSEC)
#else
	#define PCSleepMsec(_MSEC)	usleep((_MSEC) * 1000)
#endif

//���ִ������Ӧ��
#if defined (_WIN32)
	#define PC_SYS_ERRNO_FILENOTFIND 			ERROR_FILE_NOT_FOUND		//�ļ�������
#else
	#define PC_SYS_ERRNO_FILENOTFIND 			ENOENT
#endif

//��̬�����
#if defined (_WIN32)
	#define PCDllHandle 			HINSTANCE					//��̬����
	#define PCOpenDll(_DLLNAME) 	(::LoadLibrary(_DLLNAME))	//���ض�̬��
	#define PCCheckDllErr(_POINT)   (!_POINT)					//��鶯̬�����ʱ���ȡ��ַʱ�Ƿ����˴��󣬷����˴��󷵻�true
	#define PCGetDllFunAddr 		(::GetProcAddress)			//��ȡ��ַ
	#define PCCloseDll 				(::FreeLibrary)				//�ͷŶ�̬��
#else
	#define PCDllHandle 			void*
	#define PCOpenDll(_DLLNAME) 	(dlopen(_DLLNAME,RTLD_NOW))
	#define PCCheckDllErr(_POINT) 	(dlerror() ? true : false)	
	#define PCGetDllFunAddr 		(dlsym)
	#define PCCloseDll 				(dlclose)
#endif

//�ݹ���
#if defined (_WIN32)
	#define PC_REC_MUTEX_HANDLE		CRITICAL_SECTION			
	#define PC_REC_MUTEX_LOCK		EnterCriticalSection		
	#define PC_REC_MUTEX_UNLOCK		LeaveCriticalSection		
	#define PC_REC_MUTEX_DESTROY	DeleteCriticalSection		
#else
	#define PC_REC_MUTEX_HANDLE		pthread_mutex_t
	#define PC_REC_MUTEX_LOCK		pthread_mutex_lock
	#define PC_REC_MUTEX_UNLOCK		pthread_mutex_unlock
	#define PC_REC_MUTEX_DESTROY	pthread_mutex_destroy
#endif

//��������
#if defined (_WIN32)
	#define PC_CONDITION_HANDLE		CONDITION_VARIABLE			
	#define PC_CONDITION_NOTIFY		WakeConditionVariable		
	#define PC_CONDITION_NOTIFYALL	WakeAllConditionVariable		
	#define PC_CONDITION_DESTROY(_X)	
#else
	#define PC_CONDITION_HANDLE		pthread_cond_t
	#define PC_CONDITION_NOTIFY		pthread_cond_signal
	#define PC_CONDITION_NOTIFYALL	pthread_cond_broadcast
	#define PC_CONDITION_DESTROY(_X) pthread_cond_destroy(_X)
#endif

//SOCKET����
#if defined (_WIN32)
	#define PC_SOCKET				SOCKET				//�׽��ֶ���
	#define PC_INVALID_SOCKET		INVALID_SOCKET		//�Ƿ��׽���
	#define PC_SOCKET_ERROR			SOCKET_ERROR		//SOCKET����

	#define PCCloseSocket			closesocket			//�ر��׽���
#else
	#define PC_SOCKET				int
	#define PC_INVALID_SOCKET		(-1)
	#define PC_SOCKET_ERROR			(-1)

	#define PCCloseSocket			close
#endif

/*----------------------------���ص�ϵͳ�궨�塢���ͷ�ļ�---------------------*/

//��ƽ̨
#if defined (_WIN32)
	//ϵͳ�궨��
	#define NO_WARN_MBCS_MFC_DEPRECATION	//ѹ��MFC�ľ���
	#define _WINSOCKAPI_					//���ͷ�ļ�Windows.h��Winsock2.h���ܹ�������⣬��ΪWindows.h������Winsock.h
	#pragma warning(disable:4996)			//ѹ��sprintf�ȵľ���
	#define NOMINMAX						//������VC�в�������ʹ��std::min std::max

	//��
	#pragma comment(lib, "User32.lib")
	#pragma comment(lib, "Advapi32.lib")
	#pragma comment(lib, "Gdi32.lib")
	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "Crypt32.lib")
	#pragma comment(lib, "shlwapi.lib")
	#pragma comment(lib, "iphlpapi.lib")
	#pragma comment(lib, "Psapi.lib")
	#pragma comment(lib, "Userenv.lib")
	
	//ͷ�ļ�
	#include <Winsock2.h>
	#include <Windows.h>
	#include <atlconv.h>
	#include <sys/timeb.h>
	#include <process.h> 
	#include <direct.h> 
	#include <Shlwapi.h>
	#include <io.h>
	#include <winsvc.h>
	#include <rpcsal.h>
	#include <netfw.h>
	#include <WS2tcpip.h>
	
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <arpa/inet.h> 
	#include <sys/epoll.h>  
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <sys/socket.h>  
	#include <arpa/inet.h>
	#include <sys/stat.h> 
	#include <sys/time.h> 
	#include <dirent.h>
	#include <iconv.h>  
	#include <pthread.h>
	#include <dlfcn.h>
	#include <signal.h>
	#include <ftw.h>
	#include <sys/syscall.h>
#endif

//ͨ��
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <time.h>  
#include <errno.h>

//STL
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <regex>

//PCLIB���ڲ���C�ӿ�ͷ�ļ�
#include "PCUtilCheckSum.h"
#include "PCUtilFirewall_Win.h"
#include "PCUtilMisc_Linux.h"
#include "PCUtilNetwork.h"
#include "PCUtilString.h"
#include "PCUtilSymEncrypt.h"
#include "PCUtilSystem.h"
#include "PCUtilRandom.h"

//ZLIB
#include "../zlib-1.2.8/zlib.h"

//OPENSSL
#include "openssl/des.h"
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "openssl/aes.h"
#include "openssl/rsa.h"
#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/ecdh.h"
#include "openssl/evp.h"
#include "openssl/objects.h"
#include "openssl/err.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/pkcs7.h"
#include "openssl/objects.h"
#include "openssl/hmac.h"
#include "openssl/ssl.h"
#include "openssl/bio.h"
#if defined (_WIN32)
	//ֻ��windows�²ż���openssl��lib
	#if defined (_DEBUG)  
		#if defined (_WIN64)  
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mtd_x64/libeay32.lib")
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mtd_x64/ssleay32.lib")
		#else  
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mtd_x86/libeay32.lib")
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mtd_x86/ssleay32.lib")
		#endif  
	#else	/*(RELEASE)*/	
		#if defined (_WIN64)  
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mt_x64/libeay32.lib")
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mt_x64/ssleay32.lib")
		#else  
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mt_x86/libeay32.lib")
			#pragma comment(lib,"openssl-1.0.1t/lib_vc12_mt_x86/ssleay32.lib")
		#endif  
	#endif	/*!(_DEBUG)*/
#endif	/*!(_WIN32)*/



//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		���ɿ����࣬���ڷ�ֵ���͵Ķ�����඼Ӧ�ü̳д��࣬�����޺�
*/
class CPCNoCopyable
{
protected:
	CPCNoCopyable() {}
	~CPCNoCopyable() {}
private:
	CPCNoCopyable(const CPCNoCopyable&);
	CPCNoCopyable& operator=(const CPCNoCopyable&);
};

/**
*@brief		�����࣬����Ϊ���ȳ�ʼ������
*			�����桿ʹ��Լ�����ⲿͳһʹ��g_Debug���󣬲��������й��챾��Ķ���
*			��ע�⡿��������ȫ������������ʹ�ÿ�������κδ��롣
*			��ע�⡿��־λ����ʱ�ڴ�����д����
*/
class CPCDebug :CPCNoCopyable
{
public:
	explicit CPCDebug();
	virtual ~CPCDebug();

public:
	void WriteLibLogFmt(const char* pFuncName, unsigned long ulLine, const char* pszLevel, const char* pszFmt, ...);

	static int  PCGetSysTime(int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, int &nMilliSecond);
	static int  PCGetAppName(char* szAppName);
	static unsigned int PCRandUInt();
private:
	bool PCOpenFile(FILE* &pLibLogFile, char pszTimeBuf[PC_MAX_PATH]);
	static char m_pszLogDir[PC_MAX_PATH];
	static bool m_bUsedOnceAtLeast;
public:
	//OPENSSL�Ķ��̻߳ص����ã��ⲿ��Ҫʹ��
	static PC_REC_MUTEX_HANDLE	*m_lock_cs;
    static void ExitPCLib(int nExitCode);
	static void CleanPCLib();
	static void PCInitRecMutex(PC_REC_MUTEX_HANDLE* mutex);
	static void PCSSL_ThreadID_CallBack(CRYPTO_THREADID* id);
	static void PCSSL_Lock_CallBack(int mode, int type, const char *file, int line);
};
extern CPCDebug g_PCDebug;

//�Ȿ���Ĵ�����־�Ͷ��ԣ�����־�ⲻͬ����־�����ṩ���ⲿ�õģ�����־λ�����Ҫ�޸ģ�ע���޸�CPCDebug���ļ��д�������
#define PC_ERROR_LOG(_logFmt, ...)					g_PCDebug.WriteLibLogFmt(__FUNCTION__,__LINE__,"ERROR",_logFmt, ## __VA_ARGS__)
#define PC_LOG_ASSERT(_Expression,_logFmt,...)		if(!(_Expression)){g_PCDebug.WriteLibLogFmt(__FUNCTION__,__LINE__,"FATAL",_logFmt, ## __VA_ARGS__);exit(1);} 
//�Ȿ���ĵ�����־�������汾ʱ��Ҫ�ص�
#define PC_DEBUG_LOG(_logFmt, ...)					g_PCDebug.WriteLibLogFmt(__FUNCTION__,__LINE__,"DEBUG",_logFmt, ## __VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////

#endif /*_PCLIB__H_*/