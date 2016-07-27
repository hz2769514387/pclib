#include "PC_Lib.h"
#include "PCLock.h" 
#include "PCConfig.h" 
#include "PCUtilSystem.h"
#include "PCUtilString.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


CPCConfig::CPCConfig()
{
}

CPCConfig::~CPCConfig()
{
}

int CPCConfig::GetCfg(const char * pszSec, const char* pszKey, char * pValueBuf, unsigned int nValueBufLen, const char * pszFilePath)
{
	if (pszSec == NULL || pszKey == NULL || pValueBuf == NULL || pszSec[0] == 0 || pszKey[0] == 0 || nValueBufLen < 1 || NULL == pszFilePath)
	{
		PC_ERROR_LOG("params err! pszSec=%s, pszKey=%s, nValueBufLen=%u, pszFilePath=%s", pszSec, pszKey, nValueBufLen, pszFilePath);
		return PC_RESULT_PARAM;
	}
	CPCGuard guard(m_Mutex);

	//���ļ�
	FILE* pConfigFile = fopen(pszFilePath, "r");
	if (NULL == pConfigFile)
	{
		//����
		PC_ERROR_LOG("openfile read fail! filepath = %s", pszFilePath);
		return PC_RESULT_FILEOPEN;
	}

	//���ж�ȡ�������ļ�
	char szLine[PC_CFG_LINE_MAX_LEN] = { 0 };
	std::pair<string, string> pairCurr;
	string strCurrSection = "";
	while (fgets(szLine, PC_CFG_LINE_MAX_LEN, pConfigFile) != NULL)
	{
		char * pRealLine = PCStrTrim(szLine, " \n\t\r");
		int nRealLineLen = (int)strlen(pRealLine);
		if (pRealLine[0] == 0 || pRealLine[0] == PC_CFG_CMT_CHAR)
		{
			//���л�ע��
			continue;
		}
		else if (pRealLine[0] == '[' && pRealLine[nRealLineLen - 1] == ']')
		{
			//����
			pRealLine[nRealLineLen - 1] = '\0';
			strCurrSection = pRealLine + 1;
			continue;
		}
		else
		{
			//ʵ������
			int nPos = -1;
			for (int i = 0; i < nRealLineLen; i++)
			{
				if (pRealLine[i] == '=')
				{
					nPos = i;
					pRealLine[i] = '\0';
					break;
				}
			}
			if (nPos == -1 || nPos == 0)
			{
				//û�ҵ��ȺŻ�Ⱥ��ڵ�һ���ַ�
				fclose(pConfigFile);
				PC_ERROR_LOG("find = err! nPos = %d, pRealLine=%s", nPos, pRealLine);
				return PC_RESULT_FORMATERROR;
			}
			pairCurr.first = PCStrTrim(pRealLine, " \n\t\r");
			pairCurr.second = PCStrTrim(pRealLine + nPos + 1, " \n\t\r");
		}

		//����ҵ���
		if (strCurrSection == pszSec && pairCurr.first == pszKey)
		{
			fclose(pConfigFile);
			if (pairCurr.second.length() >= nValueBufLen)
			{
				PC_ERROR_LOG("buf too short! value=%s,  nValueBufLen=%u", pairCurr.second.c_str(), nValueBufLen);
				return PC_RESULT_PARAM;
			}
			strcpy(pValueBuf, pairCurr.second.c_str());
			return PC_RESULT_SUCCESS;
		}
	}

	fclose(pConfigFile);
	PC_ERROR_LOG("params err NOT FIND! pszSec=%s, pszKey=%s, nValueBufLen=%u", pszSec, pszKey, nValueBufLen);
	return PC_RESULT_PARAM;
}

int CPCConfig::SetCfg(const char * pszSec, const char* pszKey, const char * pszValue, const char * pszFilePath)
{
	CPCGuard guard(m_Mutex);

	int nRet = OpenFileAndParseToMap(pszFilePath);
	if (nRet != PC_RESULT_SUCCESS)
	{
		return nRet;
	}

	nRet = SetCfgToMap(pszSec, pszKey, pszValue);
	if (nRet != PC_RESULT_SUCCESS)
	{ 
		return nRet;
	}

	return  FlushMapToFile( pszFilePath);
}

int CPCConfig::GetSection(const char * pszSec, std::map<string,string>& mKeyValue, const char * pszFilePath)
{
    if (NULL == pszSec || NULL == pszFilePath)
    {
        PC_ERROR_LOG("params err!pszSec=%s,pszFilePath=%s", pszSec, pszFilePath);
        return PC_RESULT_PARAM;
    }
    CPCGuard guard(m_Mutex);

    int nRet = OpenFileAndParseToMap(pszFilePath);
    if (nRet != PC_RESULT_SUCCESS)
    {
        return nRet;
    }

    return CopySectionMap(pszSec,mKeyValue);
}

int CPCConfig::SetCfgToMap(const char * pszSec, const char* pszKey, const char * pszValue)
{
	if (pszSec == NULL || pszKey == NULL || pszValue == NULL || pszSec[0] == 0 || pszKey[0] == 0 )
	{
		PC_ERROR_LOG("params err! pszSec=%s, pszKey=%s", pszSec, pszKey);
		return PC_RESULT_PARAM;
	}
	
	//���ļ�ӳ���map����
    for (PC_CFG_FILE_MAP::iterator itSec = m_CfgMap.begin(); itSec != m_CfgMap.end(); itSec++)
	{
		//�ҵ�section
		if ((*itSec).first == pszSec)
		{
			//���²���key
			for (PC_ORDERED_MAP::iterator itKV = (*itSec).second.begin(); itKV != (*itSec).second.end(); itKV++)
			{
				//�ҵ�key������value
				if ((*itKV).first == pszKey)
				{
					(*itKV).second = pszValue;
					return PC_RESULT_SUCCESS;
				}
			}
			//û�ҵ�key���½�key=value
			std::pair<string, string> pairKeyValue;
			pairKeyValue.first = pszKey;
			pairKeyValue.second = pszValue;
			(*itSec).second.push_back(pairKeyValue);
			return PC_RESULT_SUCCESS;
		}
	}

	//û�ҵ�section���½�section��key=value
	std::pair<string, string> pairKeyValue;
	pairKeyValue.first = pszKey;
	pairKeyValue.second = pszValue;

	PC_ORDERED_MAP lstCurrSec;
	lstCurrSec.push_back(pairKeyValue);

	std::pair<string, PC_ORDERED_MAP> pairSec;
	pairSec.first = pszSec;
	pairSec.second = lstCurrSec;
    m_CfgMap.push_back(pairSec);
	return PC_RESULT_SUCCESS;
}

int CPCConfig::OpenFileAndParseToMap(const char * pszFilePath)
{
	if (NULL == pszFilePath)
	{
		PC_ERROR_LOG("params err!pszFilePath=%s",  pszFilePath);
		return PC_RESULT_PARAM;
	}

	//���
    m_CfgMap.clear();

    //���ļ�
    FILE* fp = fopen(pszFilePath, "r");
    if (NULL == fp)
	{
		if (PC_SYS_ERRNO_FILENOTFIND == PCGetLastError())
		{
			//�ļ�������ʱ��Ҳ��Ϊ�ɹ�����Ϊ�ǽ��ļ����ݽ�����һ���յ�map��
			PC_DEBUG_LOG("file not exist! filepath = %s", pszFilePath);
			return PC_RESULT_SUCCESS;
		}
		else
		{
			//����
			PC_ERROR_LOG("openfile read fail! filepath = %s", pszFilePath);
			return PC_RESULT_FILEOPEN;
		}
	}
    std::shared_ptr<FILE> pConfigFilePtr(fp,fclose);
	
	//���ж�ȡ�������ļ�
    char szLine[PC_CFG_LINE_MAX_LEN] = { 0 };
	std::pair<string, string> pairLine;	//��ǰ��
	string strCurrSecName = "";			//��ǰsection����
	PC_ORDERED_MAP lstCurrSec;			//��ǰsection����
    while (fgets(szLine, PC_CFG_LINE_MAX_LEN, pConfigFilePtr.get()) != NULL)
	{
		char * pRealLine = PCStrTrim(szLine, " \n\t\r");
        size_t nRealLineLen = strlen(pRealLine);
		if (pRealLine[0] == 0 || pRealLine[0] == PC_CFG_CMT_CHAR)
		{
			//���л�ע��
			pairLine.first = "";
			pairLine.second = pRealLine;
		}
		else if (pRealLine[0] == '[' && pRealLine[nRealLineLen - 1] == ']')
		{
			//����һ�ڵ�����д��ȥ
			if (lstCurrSec.size() > 0)
			{
				std::pair<string, PC_ORDERED_MAP> pairCurrSec;
				pairCurrSec.first = strCurrSecName;
				pairCurrSec.second = lstCurrSec;
                m_CfgMap.push_back(pairCurrSec);
			}
			lstCurrSec.clear();

			//��ǰ����
			pRealLine[nRealLineLen - 1] = '\0';
			strCurrSecName = pRealLine + 1;
			continue;
		}
		else
		{
			//ʵ������
			int nPos = -1;
            for (size_t i = 0; i < nRealLineLen; i++)
			{
				if (pRealLine[i] == '=')
				{
					nPos = i;
					pRealLine[i] = '\0';
					break;
				}
			}
			if (nPos == -1 || nPos == 0)
			{
                //û�ҵ��ȺŻ�Ⱥ��ڵ�һ���ַ�
				PC_ERROR_LOG("find = err! nPos = %d", nPos);
				return PC_RESULT_FORMATERROR;
			}
			pairLine.first = PCStrTrim(pRealLine, " \n\t\r");
			pairLine.second = PCStrTrim(pRealLine + nPos + 1, " \n\t\r");
		}
		lstCurrSec.push_back(pairLine);
	}

	//�����һ�ڵ�����д��ȥ
	if (lstCurrSec.size() > 0)
	{
		std::pair<string, PC_ORDERED_MAP> pairCurrSec;
		pairCurrSec.first = strCurrSecName;
		pairCurrSec.second = lstCurrSec;
        m_CfgMap.push_back(pairCurrSec);
	}
	
	return PC_RESULT_SUCCESS;
}

int CPCConfig::CopySectionMap(const char * pszSec, std::map<string,string>& mKeyValue)
{
    if (NULL == pszSec )
    {
        PC_ERROR_LOG("params err!pszSec=null");
        return PC_RESULT_PARAM;
    }

    //���ڲ��������ļ�map������mKeyValue
    for (PC_CFG_FILE_MAP::iterator itSec = m_CfgMap.begin(); itSec != m_CfgMap.end(); itSec++)
    {
        //�ҵ�section
        if ((*itSec).first == pszSec)
        {
            //����key
            for (PC_ORDERED_MAP::iterator itKV = (*itSec).second.begin(); itKV != (*itSec).second.end(); itKV++)
            {
                if("" == (*itKV).first)
                {
                    continue;
                }
                mKeyValue[(*itKV).first] = (*itKV).second;
            }
            return PC_RESULT_SUCCESS;
        }
    }

    //û�ҵ�Section
    PC_ERROR_LOG("not find section:%s ", pszSec);
    return PC_RESULT_PARAM;
}

int CPCConfig::FlushMapToFile(const char * pszFilePath)
{
	if (NULL == pszFilePath)
	{
		PC_ERROR_LOG("params err!pszFilePath=%s", pszFilePath);
		return PC_RESULT_PARAM;
	}

	FILE* pConfigFile = fopen(pszFilePath, "w");
	if (NULL == pConfigFile)
	{
		//����
		PC_ERROR_LOG("openfile write fail! filepath = %s", pszFilePath);
		return PC_RESULT_FILEOPEN;
	}

    for (PC_CFG_FILE_MAP::iterator itSec = m_CfgMap.begin(); itSec != m_CfgMap.end(); itSec++)
	{
		if ((*itSec).first != "")
		{
			fputs("[", pConfigFile);
		}
		fputs((*itSec).first.c_str(), pConfigFile);
		if ((*itSec).first != "")
		{
			fputs("]", pConfigFile);
		}
		fputs("\r\n", pConfigFile);
		for (PC_ORDERED_MAP::iterator itKV = (*itSec).second.begin(); itKV != (*itSec).second.end(); itKV++)
		{
			if ((*itKV).first != "")
			{
				fputs((*itKV).first.c_str(), pConfigFile);
				fputs("=", pConfigFile);
			}
			fputs((*itKV).second.c_str(), pConfigFile);
			fputs("\r\n", pConfigFile);
		}
	}
	fclose(pConfigFile);
	return PC_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
