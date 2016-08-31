#if !defined(_PCUTILSERVICEWIN_H_)
#define _PCUTILSERVICEWIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#if defined (_WIN32)


/**
*@brief		windows服务操作封装类，需要管理员权限
*/
class CPCWinServicesManage :CPCNoCopyable
{
public:
	explicit CPCWinServicesManage(const char *  pszServiceName);
	virtual ~CPCWinServicesManage();

	//服务控制函数
	int  CreateUserSvc(const char *  pszBinaryPath, const char *  pszDisplayName = NULL);
	int  StartSvc();
	int  StopSvc();
	int  RemoveSvc();
	/**
	*@brief		服务状态查询
	*@param		无
	*@return	成功则返回如下服务状态；失败返回 < 0 的错误码，见PC_Lib.h
	*@			SERVICE_STOPPED				1
	*@			SERVICE_START_PENDING		2
	*@			SERVICE_STOP_PENDING		3
	*@			SERVICE_RUNNING				4
	*@			SERVICE_CONTINUE_PENDING	5
	*@			SERVICE_PAUSE_PENDING		6
	*@			SERVICE_PAUSED				7
	*/
	int  QuerySvcStats();	

	//完整服务创建函数
	int  CreateSvc(const char *  pszBinaryPath, const char *  pszDisplayName, DWORD dwServiceType, DWORD dwStartType);
private:

	//服务管理器句柄，对象构造时打开，对象析构时关闭
	SC_HANDLE	m_hSCManager;
	char		m_pszServiceName[PC_MAX_PATH];
};



/************************************************************************************
*将你的程序改为服务程序																*
*************************************************************************************/

//启动和停止的回调函数定义，返回值代表错误码，见PC_Lib.h
//typedef int (*PCWinSvcCallBack)(void);
typedef std::function<int(void)> PCWinSvcCallBack;

/**
*@brief		将你的程序改为服务程序
*/
class CPCWinServiceInstance :CPCNoCopyable
{
public:
	/**
	*@brief		服务程序入口。调用者调用这个函数后，服务便从启动回调函数运行。
	*@param		pszServiceName		[IN]	服务名称
	*@param		lpfnStartCallBack	[IN]	启动回调函数，服务启动后会运行传入的启动回调函数
	*@param		lpfnStopCallBack	[IN]	停止回调函数，服务收到停止信号后会运行传入的停止回调函数
	*@return	错误码，见PC_Lib.h
	*/
	static int  ServiceEntry(const char * pszServiceName, PCWinSvcCallBack lpfnStartCallBack, PCWinSvcCallBack lpfnStopCallBack);
	
private:
	explicit CPCWinServiceInstance(){}
	virtual ~CPCWinServiceInstance(){}

	static void __stdcall PCWinNTServiceMain(DWORD dwNumServicesArgs, LPSTR  *lpServiceArgVectors);
	static void __stdcall PCServiceControlHander(DWORD dwControl);
	
	static char	m_pszServiceName[PC_MAX_PATH];
	static PCWinSvcCallBack	m_NTSvcStartCallBack;
	static PCWinSvcCallBack	m_NTSvcStopCallBack;
	static SERVICE_STATUS		m_status;
	static SERVICE_STATUS_HANDLE m_hServiceStatus;
	static CPCRecursiveLock		m_Mutex;
};



class CPCServiceAppDemo
{
public:
	static CPCServiceAppDemo * GetInstance()
	{
		static CPCServiceAppDemo m_me;
		return &m_me;
	}

	int StartApp(const char * pSevName)
	{
		return CPCWinServiceInstance::ServiceEntry(pSevName,
			std::bind(&CPCServiceAppDemo::StartSev, this),
			std::bind(&CPCServiceAppDemo::StopSev, this));
	}

	virtual int StartSev(void)
	{
		m_running = true;
		m_stoped = false;
		while (m_running)
		{
			PC_DEBUG_LOG( "I am running!!");
			PCSleepMsec(1000);
		}

		m_stoped = true;
		PC_DEBUG_LOG( "I am Stoped!!");
		return 1;
	}
	virtual int StopSev(void)
	{
		m_running = false;
		while (false == m_stoped)
		{
			PCSleepMsec(10);
		}
		PC_DEBUG_LOG("StopFunc I am Stoped!!");
		return 2;
	}
protected:
	CPCServiceAppDemo(){}
	virtual ~CPCServiceAppDemo(){}
	int m_running;
	int m_stoped;
};



#endif	// !defined(_WIN32)

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILSERVICEWIN_H_)
