#ifndef _PCCONFIG_H_
#define _PCCONFIG_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//�����ļ�ע�͵Ŀ�ͷ�ַ�
#define PC_CFG_CMT_CHAR			';'

//�����ļ�һ�е���󳤶�
#define PC_CFG_LINE_MAX_LEN		(512)

//����˳�����е�MAP
using namespace std;
typedef std::vector<std::pair<string, string> >			PC_ORDERED_MAP;
typedef std::vector<std::pair<string, PC_ORDERED_MAP> >	PC_CFG_FILE_MAP;

/**
*@brief		�����ļ��࣬һ��CPCConfig�����ȡһ�������ļ�
*/
class CPCConfig :CPCNoCopyable
{
public:
	explicit CPCConfig();
    virtual ~CPCConfig();

	//��д�����ļ�
	int GetCfg(const char * pszSec, const char* pszKey, char * pValueBuf, unsigned int nValueBufLen, const char * pszFilePath);
	int SetCfg(const char * pszSec, const char* pszKey, const char * pszValue, const char * pszFilePath);
	
    //��ȡ����Section�����м�ֵ��,�������mKeyValueԭ�����ݣ�������ͬkey�����ݻᱻ����
    int GetSection(const char * pszSec, std::map<string,string>& mKeyValue, const char * pszFilePath);
protected:
	//�ֲ���д�����ļ����ڲ�����δ����
	int OpenFileAndParseToMap(const char * pszFilePath);
	int SetCfgToMap(const char * pszSec, const char* pszKey, const char * pszValue);
    int CopySectionMap(const char * pszSec, std::map<string,string>& mKeyValue);
	int FlushMapToFile(const char * pszFilePath);

	//���ڿ���Ҫ��д��������ļ�������δ�þ�̬��
    CPCRecursiveLock    m_Mutex;

private:

	//section name, lines��map�����ĳ��Ԫ�ص�section name  = ""�����������ͷ��û��section�Ĳ��֣���Ԫ�ؿ϶���ע�ͻ����
	//lines�ĽṹΪkey=value�����key=""�������Ϊ���л�ע��
    PC_CFG_FILE_MAP     m_CfgMap;
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif/*_PCCONFIG_H_*/
