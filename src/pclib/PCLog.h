#ifndef _PCLOG_H_
#define _PCLOG_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//һ����־��󳤶ȣ�4MB
#define PC_LOG_LINE_MAX_LEN		(4*1024*1024)

//�Ƿ�����д��
#define PC_LOG_WRITE_ALWAYS		(false)


/**
*@brief		��־��
*/
class CPCLog :CPCNoCopyable
{
public:
	virtual ~CPCLog();
	static CPCLog* GetRoot();

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
	*@param		nLevel	[IN]	������־����־�ȼ�
	*@param		pszFmt	[IN]	��־���ݸ�ʽ��/��־����
	*@return	�����룬��PCDebug.h
	*/
	int WriteLogFmt(int nLevel, const char* pszFmt, ...);
	/**
	*@brief		д������������־����־д��Ϻ���ӻ��з�
	*@param		nLevel		[IN]	������־����־�ȼ�
	*@param		pszTips		[IN]	ǰ����ʾ�ַ���
	*@param		pszBytes	[IN]	������־����
	*@param		nBytesLen	[IN]	������־���ݳ���
	*@return	�����룬��PC_Lib.h
	*/
	int WriteLogBytes(int nLevel, const char* pszTips, const unsigned char* pszBytes, unsigned int nBytesLen);
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


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif /*_PCLOG_H_*/
