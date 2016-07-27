#ifndef _PCCONFIG_H_
#define _PCCONFIG_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//配置文件注释的开头字符
#define PC_CFG_CMT_CHAR			';'

//配置文件一行的最大长度
#define PC_CFG_LINE_MAX_LEN		(512)

//按照顺序排列的MAP
using namespace std;
typedef std::vector<std::pair<string, string> >			PC_ORDERED_MAP;
typedef std::vector<std::pair<string, PC_ORDERED_MAP> >	PC_CFG_FILE_MAP;

/**
*@brief		配置文件类，一个CPCConfig对象读取一个配置文件
*/
class CPCConfig :CPCNoCopyable
{
public:
	explicit CPCConfig();
    virtual ~CPCConfig();

	//读写配置文件
	int GetCfg(const char * pszSec, const char* pszKey, char * pValueBuf, unsigned int nValueBufLen, const char * pszFilePath);
	int SetCfg(const char * pszSec, const char* pszKey, const char * pszValue, const char * pszFilePath);
	
    //读取整个Section的所有键值对,不会清空mKeyValue原有数据，但是相同key的数据会被覆盖
    int GetSection(const char * pszSec, std::map<string,string>& mKeyValue, const char * pszFilePath);
protected:
	//分步读写配置文件，内部函数未加锁
	int OpenFileAndParseToMap(const char * pszFilePath);
	int SetCfgToMap(const char * pszSec, const char* pszKey, const char * pszValue);
    int CopySectionMap(const char * pszSec, std::map<string,string>& mKeyValue);
	int FlushMapToFile(const char * pszFilePath);

	//由于可能要读写多个配置文件，所以未用静态锁
    CPCRecursiveLock    m_Mutex;

private:

	//section name, lines的map，如果某个元素的section name  = ""，则其属于最开头的没有section的部分，此元素肯定是注释或空行
	//lines的结构为key=value，如果key=""，则此行为空行或注释
    PC_CFG_FILE_MAP     m_CfgMap;
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif/*_PCCONFIG_H_*/
