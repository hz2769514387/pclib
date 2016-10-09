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
*@brief		日志类
*			[警告]日志类使用了pclib中的锁、日期时间处理、文件夹处理的函数，所以pclib中这些函数不能写日志，以免产生死循环
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

	//日志等级
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

	//日志生成模式，按天生成或按小时生成
	enum ePCLogGenMode
	{
		eGenModeHour = 1,
		eGenModeDay
	};

public:
	/**
	*@brief		设置写日志属性
	*@param		nLevel		[IN]	设定的日志等级，低于日志等级的日志不写，参考PCDebug.h
	*@param		nGenMode	[IN]	日志生成模式，参考PCDebug.h
	*@param		bStdout		[IN]	是否将日志打印到控制台
	*@param		pszLogPath	[IN]	日志文件路径。默认为当前目录下的logs目录
	*@return	错误码，见PC_Lib.h
	*/
	int	SetLogAttr(int nLevel, int nGenMode, bool bStdout, const char * pszLogPath);

	/**
	*@brief		写日志，日志写完毕后添加换行符
	*@param		pFuncName	[IN]	当条日志的函数名称，可为空
	*@param		ulLine		[IN]	当条日志在文件中的行数
	*@param		nLevel		[IN]	当条日志的日志等级
	*@param		pszFmt		[IN]	日志内容格式串/日志内容
	*@return	错误码，见PCDebug.h
	*/
	int WriteLogFmt(const char* pFuncName, unsigned long ulLine, int nLevel, const char* pszFmt, ...);
	/**
	*@brief		写二进制数据日志，日志写完毕后添加换行符
	*@param		pFuncName	[IN]	当条日志的函数名称，可为空
	*@param		ulLine		[IN]	当条日志在文件中的行数
	*@param		nLevel		[IN]	当条日志的日志等级
	*@param		pszTips		[IN]	前导提示字符串
	*@param		pszBytes	[IN]	数据日志内容
	*@param		nBytesLen	[IN]	数据日志内容长度
	*@return	错误码，见PC_Lib.h
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

	//当前文件的年月日。
	char m_pszCurrFileTime[PC_MAX_PATH];
};

//日志宏
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
