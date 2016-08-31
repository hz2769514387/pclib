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
*@brief		windows���������װ�࣬��Ҫ����ԱȨ��
*/
class CPCWinServicesManage :CPCNoCopyable
{
public:
	explicit CPCWinServicesManage(const char *  pszServiceName);
	virtual ~CPCWinServicesManage();

	//������ƺ���
	int  CreateUserSvc(const char *  pszBinaryPath, const char *  pszDisplayName = NULL);
	int  StartSvc();
	int  StopSvc();
	int  RemoveSvc();
	/**
	*@brief		����״̬��ѯ
	*@param		��
	*@return	�ɹ��򷵻����·���״̬��ʧ�ܷ��� < 0 �Ĵ����룬��PC_Lib.h
	*@			SERVICE_STOPPED				1
	*@			SERVICE_START_PENDING		2
	*@			SERVICE_STOP_PENDING		3
	*@			SERVICE_RUNNING				4
	*@			SERVICE_CONTINUE_PENDING	5
	*@			SERVICE_PAUSE_PENDING		6
	*@			SERVICE_PAUSED				7
	*/
	int  QuerySvcStats();	

	//�������񴴽�����
	int  CreateSvc(const char *  pszBinaryPath, const char *  pszDisplayName, DWORD dwServiceType, DWORD dwStartType);
private:

	//��������������������ʱ�򿪣���������ʱ�ر�
	SC_HANDLE	m_hSCManager;
	char		m_pszServiceName[PC_MAX_PATH];
};



/************************************************************************************
*����ĳ����Ϊ�������																*
*************************************************************************************/

//������ֹͣ�Ļص��������壬����ֵ��������룬��PC_Lib.h
//typedef int (*PCWinSvcCallBack)(void);
typedef std::function<int(void)> PCWinSvcCallBack;

/**
*@brief		����ĳ����Ϊ�������
*/
class CPCWinServiceInstance :CPCNoCopyable
{
public:
	/**
	*@brief		���������ڡ������ߵ�����������󣬷����������ص��������С�
	*@param		pszServiceName		[IN]	��������
	*@param		lpfnStartCallBack	[IN]	�����ص���������������������д���������ص�����
	*@param		lpfnStopCallBack	[IN]	ֹͣ�ص������������յ�ֹͣ�źź�����д����ֹͣ�ص�����
	*@return	�����룬��PC_Lib.h
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
