#ifndef _PCLIB__H_
#define _PCLIB__H_
#if defined(_MSC_VER)
# pragma once
#endif

/*----------------------------接口统一错误码-----------------------------------*/
#define PC_RESULT_SUCCESS			( 0)	//成功
#define	PC_RESULT_PARAM				(-1)	//参数错误
#define	PC_RESULT_SYSERROR			(-2)	//系统异常
#define	PC_RESULT_BUFFNOT_ENOUGH	(-3)	//分配的空间长度不足
#define	PC_RESULT_MEMORY			(-4)	//分配内存失败
#define	PC_RESULT_FORMATERROR		(-5)	//字符串格式错误
#define	PC_RESULT_HANDLEINVALID		(-6)	//不可用的句柄
#define	PC_RESULT_FILEOPEN			(-7)	//打开文件失败
#define	PC_RESULT_FILEREAD			(-8)	//读文件出错
#define	PC_RESULT_TIMEOUT			(-9)	//操作超时
#define	PC_RESULT_NOMOREDATA		(-10)	//没有更多数据了
#define	PC_RESULT_SERVICE_NOTEXIST	(-11)	//指定的服务未安装（windows）

/*----------------------------通用宏定义---------------------------------------*/
//PCLib库版本。对于PCLib库本身，所有的输入输出字符串默认都为GBK编码，除非有特殊说明。
#define PC_LIB_VERSION			"V1.0.0.0"

//名字空间
#define PCLIB_NAMESPACE_BEG		namespace pclib {
#define PCLIB_NAMESPACE_END		}

//断言宏
#define PC_ASSERT(_Expression,_logFmt,...)	if(!(_Expression)){printf(_logFmt, ## __VA_ARGS__);abort();} 

//定义静态常量属性
#define PC_STATIC_PROPERTY(T, name, ...)	static const T name = (__VA_ARGS__)

//本地路径长度
#define PC_MAX_PATH				(260)

//URL路径长度
#define PC_MAX_URL				(2084)

//一行日志最大长度，4MB
#define PC_LOG_LINE_MAX_LEN		(4*1024*1024)

//日志是否立即写入
#define PC_LOG_WRITE_ALWAYS		(true)


/*----------------------------宏实现的部分跨平台函数---------------------------*/
//获取当前线程ID，返回ULONG,LINUX返回int
#if defined (_WIN32)
	#define PCGetCurrentThreadID()	GetCurrentThreadId()
#else
	#define PCGetCurrentThreadID()	syscall(SYS_gettid)
#endif

//获取当前进程ID，WINDOWS返回ULONG,LINUX返回int
#if defined (_WIN32)
	#define PCGetCurrentProccessID	GetCurrentProcessId
#else
	#define PCGetCurrentProccessID	getpid
#endif

//不区分大小写的字符串比较
#if defined (_WIN32)
	#define PCStrCaseCmp	_stricmp
#else
	#define PCStrCaseCmp	strcasecmp
#endif

//不区分大小写的字符串指定长度比较
#if defined (_WIN32)
	#define PCStrnCaseCmp	_strnicmp
#else
	#define PCStrnCaseCmp	strncasecmp
#endif

//获取当前的工作目录
#if defined (_WIN32)
	#define PCGetCwd	_getcwd
#else
	#define PCGetCwd	getcwd
#endif

//创建文件夹，返回值：成功0 失败-1
#if defined (_WIN32)
	#define PCMakeDir(_DIRNAME)	_mkdir(_DIRNAME)
#else
	#define PCMakeDir(_DIRNAME)	mkdir(_DIRNAME, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

//休眠指定毫秒数
#if defined (_WIN32)
	#define PCSleepMsec(_MSEC)	Sleep(_MSEC)
#else
	#define PCSleepMsec(_MSEC)	usleep((_MSEC) * 1000)
#endif

//部分错误码对应表
#if defined (_WIN32)
	#define PC_SYS_ERRNO_FILENOTFIND 			ERROR_FILE_NOT_FOUND		//文件不存在
#else
	#define PC_SYS_ERRNO_FILENOTFIND 			ENOENT
#endif

//动态库相关
#if defined (_WIN32)
	#define PCDllHandle 			HINSTANCE					//动态库句柄
	#define PCOpenDll(_DLLNAME) 	(::LoadLibrary(_DLLNAME))	//加载动态库
	#define PCCheckDllErr(_POINT)   (!_POINT)					//检查动态库加载时或获取地址时是否发生了错误，发生了错误返回true
	#define PCGetDllFunAddr 		(::GetProcAddress)			//获取地址
	#define PCCloseDll 				(::FreeLibrary)				//释放动态库
#else
	#define PCDllHandle 			void*
	#define PCOpenDll(_DLLNAME) 	(dlopen(_DLLNAME,RTLD_NOW))
	#define PCCheckDllErr(_POINT) 	(dlerror() ? true : false)	
	#define PCGetDllFunAddr 		(dlsym)
	#define PCCloseDll 				(dlclose)
#endif

//递归锁
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

//条件变量
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

//SOCKET网络
#define		PC_SOCKET_TYPE			AF_INET				//协议，可指定以下类型：
														//	AF_INET		仅支持IPv4，DNS解析速度快
														//	AF_UNSPEC	兼容IPv4和IPv6，DNS解析速度慢
#if defined (_WIN32)
	#define PC_SOCKET				SOCKET				//套接字定义
	#define PC_INVALID_SOCKET		INVALID_SOCKET		//非法套接字
	#define PC_SOCKET_ERROR			SOCKET_ERROR		//SOCKET出错

	#define PCCloseSocket(_S)		{closesocket(_S);_S=PC_INVALID_SOCKET;}	
#else
	#define PC_SOCKET				int
	#define PC_INVALID_SOCKET		(-1)
	#define PC_SOCKET_ERROR			(-1)

	#define PCCloseSocket(_S)		{close(_S);_S=PC_INVALID_SOCKET;}
#endif

/*----------------------------加载的系统宏定义、库和头文件---------------------*/

//分平台
#if defined (_WIN32)
	//系统宏定义
	#define NO_WARN_MBCS_MFC_DEPRECATION	//压制MFC的警告
	#define _WINSOCKAPI_					//解决头文件Windows.h和Winsock2.h不能共存的问题，因为Windows.h包含了Winsock.h
	#pragma warning(disable:4996)			//压制sprintf等的警告
	#define NOMINMAX						//这样在VC中才能正常使用std::min std::max

	//库
	#pragma comment(lib, "User32.lib")
	#pragma comment(lib, "Advapi32.lib")
	#pragma comment(lib, "Gdi32.lib")
	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "Crypt32.lib")
	#pragma comment(lib, "shlwapi.lib")
	#pragma comment(lib, "iphlpapi.lib")
	#pragma comment(lib, "Psapi.lib")
	#pragma comment(lib, "Userenv.lib")
	
	//头文件
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
	#include <MSWSock.h>
	
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <arpa/inet.h> 
	#include <sys/epoll.h>  
	#include <netinet/in.h>
	#include <sys/types.h>
	#include <sys/socket.h>  
    #include <netdb.h>
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

//通用
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <time.h>  
#include <errno.h>
#include <assert.h>
#include <locale>  
#include <codecvt>  
#include <fstream>  

//STL
#include <cstddef>
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
#include <atomic>
#include <limits>


//PCLIB库内部的C接口头文件
//#include "PCUtilCheckSum.h"
//#include "PCUtilFirewall_Win.h"
//#include "PCUtilMisc_Linux.h"
//#include "PCUtilMisc_Win.h"
//#include "PCUtilNetwork.h"
//#include "PCUtilString.h"
//#include "PCUtilSymEncrypt.h"
//#include "PCUtilSystem.h"


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
	//只在windows下才加载openssl的lib
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
*@brief		不可拷贝类，对于非值类型的对象的类都应该继承此类，有益无害
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
*@brief		pblib类，定义为最先初始化和最后反初始化的类
*			【注意】此类先于日志类创建，必须完全独立，不允许使用库里面的任何代码。
*/
class CPCLib :CPCNoCopyable
{
public:
	explicit CPCLib();
	virtual ~CPCLib();

public:
	//OPENSSL的多线程回调设置，外部不要使用
	static PC_REC_MUTEX_HANDLE	*m_lock_cs;
	static void PCInitRecMutex(PC_REC_MUTEX_HANDLE* mutex);
	static void PCSSL_ThreadID_CallBack(CRYPTO_THREADID* id);
	static void PCSSL_Lock_CallBack(int mode, int type, const char *file, int line);

	//Windows下的IOCP相关函数需手动加载
#if defined (_WIN32)
	static LPFN_CONNECTEX				m_lpfnConnectEx;
	static LPFN_ACCEPTEX				m_lpfnAcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSockAddrs;
#endif
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////

#endif /*_PCLIB__H_*/
