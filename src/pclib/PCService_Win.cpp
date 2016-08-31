#if defined (_WIN32)

#include "PC_Lib.h"
#include "PCLock.h" 
#include "PCUtilSystem.h"
#include "PCService_Win.h"
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


CPCWinServicesManage::CPCWinServicesManage(const char *  pszServiceName)
{
	PC_ASSERT((pszServiceName && (strlen(pszServiceName) < PC_MAX_PATH)), "pszServiceName == NULL!");
	strcpy(m_pszServiceName, pszServiceName);

	m_hSCManager = NULL;
	m_hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("OpenSCManagerA SC_MANAGER_ALL_ACCESS fail! pszServiceName=%s", pszServiceName);
	}
}

CPCWinServicesManage::~CPCWinServicesManage()
{
	if (m_hSCManager)
	{
		CloseServiceHandle(m_hSCManager);
		m_hSCManager = NULL;
	}
}

int  CPCWinServicesManage::CreateUserSvc(const char *  pszBinaryPath, const char *  pszDisplayName )
{
	return CreateSvc(pszBinaryPath, pszDisplayName, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START);
}

int  CPCWinServicesManage::StartSvc()
{
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("m_hSCManager == NULL!(m_pszServiceName=%s)!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	SC_HANDLE hService = OpenServiceA(m_hSCManager, m_pszServiceName, SERVICE_START);
	if (hService == NULL)
	{
		PC_ERROR_LOG("OpenServiceA(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	BOOL bRet = StartServiceA(hService, 0, NULL);
	CloseServiceHandle(hService);

	if (FALSE == bRet)
	{
		PC_ERROR_LOG("StartServiceA(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	return  PC_RESULT_SUCCESS;
}

int  CPCWinServicesManage::StopSvc()
{
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("m_hSCManager == NULL!(m_pszServiceName=%s)!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	SC_HANDLE hService = OpenServiceA(m_hSCManager, m_pszServiceName, SERVICE_STOP);
	if (hService == NULL)
	{
		PC_ERROR_LOG("OpenServiceA(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	SERVICE_STATUS    serviceStatus;
	BOOL bRet = ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);
	CloseServiceHandle(hService);

	if (FALSE == bRet)
	{
		PC_ERROR_LOG("ControlService STOP (m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	return  PC_RESULT_SUCCESS;
}

int  CPCWinServicesManage::RemoveSvc()
{
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("m_hSCManager == NULL!(m_pszServiceName=%s)!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}

	SC_HANDLE hService = OpenServiceA(m_hSCManager, m_pszServiceName, SERVICE_ALL_ACCESS);
	if (hService == NULL)
	{
		PC_ERROR_LOG("OpenServiceA(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}

	//先停止
	SERVICE_STATUS    serviceStatus; 
	ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

	BOOL bRet = DeleteService(hService);
	CloseServiceHandle(hService);

	if (FALSE == bRet)
	{
		PC_ERROR_LOG("DeleteService(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	return  PC_RESULT_SUCCESS;
}

int  CPCWinServicesManage::QuerySvcStats()
{
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("m_hSCManager == NULL!(m_pszServiceName=%s)!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	SC_HANDLE hService = OpenServiceA(m_hSCManager, m_pszServiceName, SERVICE_ALL_ACCESS);
	if (hService == NULL)
	{
		if (PCGetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			//服务不存在
			return PC_RESULT_SERVICE_NOTEXIST;
		}
		PC_ERROR_LOG("OpenServiceA(m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	SERVICE_STATUS    serviceStatus;
	BOOL bRet = QueryServiceStatus(hService,  &serviceStatus);
	CloseServiceHandle(hService);

	if (FALSE == bRet)
	{
		PC_ERROR_LOG("QueryServiceStatus  (m_pszServiceName=%s) fail!", m_pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	return  serviceStatus.dwCurrentState;
}


int  CPCWinServicesManage::CreateSvc(const char *  pszBinaryPath, const char *  pszDisplayName, DWORD dwServiceType, DWORD dwStartType)
{
	if ( pszBinaryPath == NULL)
	{
		PC_ERROR_LOG("params err!m_pszServiceName=%s,pszDisplayName=%s,pszBinaryPath=%s, dwServiceType = %lu, dwStartType = %lu ", m_pszServiceName, pszDisplayName, pszBinaryPath, dwServiceType, dwStartType);
		return PC_RESULT_PARAM;
	}
	if (m_hSCManager == NULL)
	{
		PC_ERROR_LOG("m_hSCManager == NULL!");
		return PC_RESULT_SYSERROR;
	}
	std::string strDisplayName = m_pszServiceName;
	if (pszDisplayName)
	{
		strDisplayName = pszDisplayName;
	}

	SC_HANDLE hService = CreateServiceA(
		m_hSCManager, 							/* SCManager database */
		m_pszServiceName, 						/* name of service */
		strDisplayName.c_str(), 				/* service name to display */
		SERVICE_ALL_ACCESS,                 	/* desired access */
		dwServiceType,							/* service type */
		dwStartType,							/* start type */
		SERVICE_ERROR_NORMAL,					/* error control type */
		pszBinaryPath,							/* service's binary */
		NULL,									/* no load ordering group */
		NULL,									/* no tag identifier */
		NULL,									/* no dependencies */
		NULL,									/* LocalSystem account */
		NULL);									/* no password */
	if (NULL == hService)
	{
		PC_ERROR_LOG("CreateServiceA fail!m_pszServiceName=%s,pszDisplayName=%s,pszBinaryPath=%s, dwServiceType = %lu, dwStartType = %lu ", m_pszServiceName, pszDisplayName, pszBinaryPath, dwServiceType, dwStartType);
		return PC_RESULT_SYSERROR;
	}

	CloseServiceHandle(hService);
	return PC_RESULT_SUCCESS;
}

/************************************************************************************
*将你的程序改为服务程序																*
*************************************************************************************/
char CPCWinServiceInstance::m_pszServiceName[PC_MAX_PATH] = {0};
PCWinSvcCallBack		CPCWinServiceInstance::m_NTSvcStartCallBack = nullptr;
PCWinSvcCallBack		CPCWinServiceInstance::m_NTSvcStopCallBack = nullptr;
SERVICE_STATUS			CPCWinServiceInstance::m_status = {};
SERVICE_STATUS_HANDLE	CPCWinServiceInstance::m_hServiceStatus = nullptr;
CPCRecursiveLock		CPCWinServiceInstance::m_Mutex;

//将你的程序改为服务模式的程序。
int  CPCWinServiceInstance::ServiceEntry(const char * pszServiceName, PCWinSvcCallBack lpfnStartCallBack, PCWinSvcCallBack lpfnStopCallBack)
{
	CPCGuard guard(m_Mutex);
	if (m_NTSvcStartCallBack != NULL)
	{
		//已经调用成功过一次了，不能重复调用
		PC_TRACE_LOG("pszServiceName (%s) is already inited!  ", pszServiceName);
		return PC_RESULT_SUCCESS;
	}
	if (NULL == pszServiceName || 0 == pszServiceName[0] || strlen(pszServiceName) >= PC_MAX_PATH || NULL == lpfnStartCallBack || NULL == lpfnStopCallBack)
	{
		PC_ERROR_LOG("params err!pszServiceName = %s ", pszServiceName);
		return PC_RESULT_PARAM;
	}
	//成员变量保存
	strcpy(m_pszServiceName, pszServiceName);
	m_NTSvcStartCallBack = lpfnStartCallBack;
	m_NTSvcStopCallBack = lpfnStopCallBack;

	//分配函数入口
	SERVICE_TABLE_ENTRYA stSrvEntry[] =
	{
		{ m_pszServiceName, (LPSERVICE_MAIN_FUNCTIONA)CPCWinServiceInstance::PCWinNTServiceMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcherA(stSrvEntry))
	{
		m_pszServiceName[0] = 0;
		m_NTSvcStartCallBack = nullptr;
		m_NTSvcStopCallBack = nullptr;
		PC_ERROR_LOG("StartServiceCtrlDispatcher err!pszServiceName = %s ", pszServiceName);
		return PC_RESULT_SYSERROR;
	}
	return PC_RESULT_SUCCESS;
}

//windows服务入口函数
void __stdcall CPCWinServiceInstance::PCWinNTServiceMain(DWORD dwNumServicesArgs, LPSTR  *lpServiceArgVectors)
{
	try
	{
		//注册服务控制
		m_hServiceStatus = RegisterServiceCtrlHandlerA(m_pszServiceName, CPCWinServiceInstance::PCServiceControlHander);
		if (m_hServiceStatus == nullptr)
		{
			PC_ERROR_LOG("RegisterServiceCtrlHandler err!m_pszServiceName = %s ", m_pszServiceName);
			return;
		}
		PC_TRACE_LOG("Service(%s) is  register success", m_pszServiceName);

		//服务准备启动
		m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		m_status.dwCurrentState = SERVICE_START_PENDING;
		m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		m_status.dwWin32ExitCode = 0;
		m_status.dwServiceSpecificExitCode = 0;
		m_status.dwCheckPoint = 0;
		m_status.dwWaitHint = 0;
		if (!SetServiceStatus(m_hServiceStatus, &m_status))
		{
			PC_ERROR_LOG("SetServiceStatus err! start fail. m_pszServiceName = %s ", m_pszServiceName);
		}

		//这里可以写一些准备启动的初始化动作

		//服务正式启动
		m_status.dwWin32ExitCode = S_OK;
		m_status.dwCheckPoint = 0;
		m_status.dwWaitHint = 0;
		m_status.dwCurrentState = SERVICE_RUNNING;
		if (!SetServiceStatus(m_hServiceStatus, &m_status))
		{
			PC_ERROR_LOG("SetServiceStatus err! start fail. m_pszServiceName = %s ", m_pszServiceName);
		}
		PC_TRACE_LOG("Service(%s) Started! ", m_pszServiceName);

		//运行提供者的回调函数
		PC_ASSERT(m_NTSvcStartCallBack, "fatal err! m_NTSvcCallBack = NULL.m_pszServiceName (%s) ", m_pszServiceName);
		int nRet = m_NTSvcStartCallBack();

		//回调函数返回，服务停止
		PC_TRACE_LOG("Service(%s) Stoped! ExitCode = %d ", m_pszServiceName, nRet);
		m_status.dwCurrentState = SERVICE_STOPPED;
		if (!SetServiceStatus(m_hServiceStatus, &m_status))
		{
			PC_ERROR_LOG("SetServiceStatus err! start fail. m_pszServiceName = %s ", m_pszServiceName);
		}
	}
	catch (...)
	{
		PC_ERROR_LOG("catch err! service run fail. m_pszServiceName = %s ", m_pszServiceName);
	}
}

//服务控制，收到这个信号的是另一个线程
void __stdcall CPCWinServiceInstance::PCServiceControlHander(DWORD dwControl)
{
	PC_ASSERT(m_hServiceStatus, "fatal err! m_hServiceStatus = NULL.m_pszServiceName(%s)", m_pszServiceName);

	int nRet = 0;
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		{
			//收到停止服务请求，回调提供者的停止函数
			nRet = m_NTSvcStopCallBack();
			m_status.dwCurrentState = SERVICE_STOPPED;
		}
		break;
	default:
		PC_ERROR_LOG("Warning! Service (%s) Recved ERROR (%lu)  Request!", m_pszServiceName, dwControl);
		return;
	}

	//状态报告
	if (!SetServiceStatus(m_hServiceStatus, &m_status))
	{
		PC_ERROR_LOG("Service(%s) Recved (%lu) Request! User func STOP ret = %d, BUT SetServiceStatus Stop FAIL!", m_pszServiceName, dwControl, nRet);
	}
	else
	{
		PC_ERROR_LOG("Service(%s) Recved (%lu) Request! User func STOP ret = %d", m_pszServiceName, dwControl, nRet);
	}
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	// !defined(_WIN32)
