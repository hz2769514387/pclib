#ifndef _PCLOG_H_
#define _PCLOG_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


/**
*@brief		��־��
*			[����]��־��ʹ����pclib�е���������ʱ�䴦���ļ��д���ĺ���������pclib����Щ��������д��־�����������ѭ��
*/
class CPCLog :CPCNoCopyable
{
public:
	virtual ~CPCLog();
	static CPCLog* GetRoot()
	{
		static CPCLog m_Log;
		return &m_Log;
	}

	//��־�ȼ�
	enum ePCLogLevel
	{
		eLevelClose = 0,
		eLevelTrace ,
		eLevelDebug ,
		eLevelInfo  ,
		eLevelWarn  ,
		eLevelError ,
		eLevelFatal
	};

	//��־����ģʽ���������ɻ�Сʱ����
	enum ePCLogGenMode
	{
		eGenModeHour = 1,
		eGenModeDay
	};

public:
	/**
	*@brief		����д��־����
	*@param		nLevel		[IN]	�趨����־�ȼ���������־�ȼ�����־��д���ο�PCDebug.h
	*@param		nGenMode	[IN]	��־����ģʽ���ο�PCDebug.h
	*@param		bStdout		[IN]	�Ƿ���־��ӡ������̨
	*@param		pszLogPath	[IN]	��־�ļ�·����Ĭ��Ϊ��ǰĿ¼�µ�logsĿ¼
	*@return	�����룬��PC_Lib.h
	*/
	int	SetLogAttr(int nLevel, int nGenMode, bool bStdout, const char * pszLogPath);

	/**
	*@brief		д��־����־д��Ϻ���ӻ��з�
	*@param		pFuncName	[IN]	������־�ĺ������ƣ���Ϊ��
	*@param		ulLine		[IN]	������־���ļ��е�����
	*@param		nLevel		[IN]	������־����־�ȼ�
	*@param		pszFmt		[IN]	��־���ݸ�ʽ��/��־����
	*@return	�����룬��PCDebug.h
	*/
	int WriteLogFmt(const char* pFuncName, unsigned long ulLine, int nLevel, const char* pszFmt, ...);
	/**
	*@brief		д������������־����־д��Ϻ���ӻ��з�
	*@param		pFuncName	[IN]	������־�ĺ������ƣ���Ϊ��
	*@param		ulLine		[IN]	������־���ļ��е�����
	*@param		nLevel		[IN]	������־����־�ȼ�
	*@param		pszTips		[IN]	ǰ����ʾ�ַ���
	*@param		pszBytes	[IN]	������־����
	*@param		nBytesLen	[IN]	������־���ݳ���
	*@return	�����룬��PC_Lib.h
	*/
	int WriteLogBytes(const char* pFuncName, unsigned long ulLine, int nLevel, const char* pszTips, const unsigned char* pszBytes, unsigned int nBytesLen);
private:
	CPCLog();
	int CheckLogger(char pszTimeBuf[PC_MAX_PATH]);

	int  m_nLogLevel;				
	int  m_nGenMode;				
	bool m_bStdout;					
	char m_pszLogPath[PC_MAX_PATH];	

	static CPCRecursiveLock m_Mutex;		
	FILE           * m_pLogFile;	
	char		   * m_pFmtBuff;	
	static const char * const m_LOG_LEVEL_NAME[];

	//��ǰ�ļ��������ա�
	char m_pszCurrFileTime[PC_MAX_PATH];
};

//��־��
#define PC_TRACE_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelTrace,_logFmt, ## __VA_ARGS__)
#define PC_DEBUG_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelDebug,_logFmt, ## __VA_ARGS__)
#define PC_INFO_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelInfo,_logFmt, ## __VA_ARGS__)
#define PC_WARN_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelWarn,_logFmt, ## __VA_ARGS__)
#define PC_ERROR_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelError,_logFmt, ## __VA_ARGS__)
#define PC_FATAL_LOG(_logFmt, ...)				CPCLog::GetRoot()->WriteLogFmt(__FUNCTION__,__LINE__,CPCLog::eLevelFatal,_logFmt, ## __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif /*_PCLOG_H_*/
