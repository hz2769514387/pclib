#include "PC_Lib.h"
#include "PCTimeValue.h"
#include "PCUtilSystem.h"
#include "PCUtilString.h"
#include "PCMemory.h" 
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


const char * const CPCLog::m_LOG_LEVEL_NAME[] = { "�ر�", "׷��", "����", "��Ϣ", "����", "����", "����" };

CPCRecursiveLock CPCLog::m_Mutex;
CPCLog::CPCLog()
    :m_nLogLevel(eLevelInfo)
    ,m_nGenMode(eGenModeDay)
    ,m_bStdout(true)
    ,m_pLogFile(NULL)

{
	//��־Ĭ������
	strcpy(m_pszLogPath, "./logs/");
	m_pFmtBuff = new (std::nothrow) char[PC_LOG_LINE_MAX_LEN];
	PC_ASSERT(m_pFmtBuff != NULL, "m_pFmtBuff�����ڴ�ʧ�ܣ�");

	memset(m_pFmtBuff, 0, PC_LOG_LINE_MAX_LEN);
	memset(m_pszCurrFileTime, 0, PC_MAX_PATH);
}

CPCLog::~CPCLog()
{
	if (m_pLogFile != NULL)
	{
		fclose(m_pLogFile);
		m_pLogFile = NULL;
	}
	if (m_pFmtBuff)
	{
		delete m_pFmtBuff;
		m_pFmtBuff = NULL;
	}
}

int CPCLog::SetLogAttr(int nLevel, int nGenMode, bool bStdout, const char * pszLogPath)
{
	if (nLevel == eLevelClose)
	{
		CPCGuard guard(m_Mutex);
		m_nLogLevel = nLevel;
		return PC_RESULT_SUCCESS;
	}
	if (nLevel < eLevelClose || nLevel > eLevelFatal ||
		nGenMode < eGenModeHour || nGenMode > eGenModeDay ||
		pszLogPath == NULL || strlen(pszLogPath) == 0 || strlen(pszLogPath) > (PC_MAX_PATH-24))
	{
		return PC_RESULT_PARAM;
	}
	size_t nLogPathLen = strlen(pszLogPath);
	CPCGuard guard(m_Mutex);

	//���õ�ʱ��Ҫ�ر��ϴδ򿪵�����ȷ��д��־״̬
	if (m_pLogFile != NULL)
	{
		fflush(m_pLogFile);
		fclose(m_pLogFile);
		m_pLogFile = NULL;
	}
	memset(m_pszCurrFileTime, 0, PC_MAX_PATH);

	m_nLogLevel = nLevel;
	m_nGenMode = nGenMode;
	m_bStdout = bStdout;
	if (pszLogPath[nLogPathLen - 1] != '/' && pszLogPath[nLogPathLen - 1] != '\\')
	{
		//��������Զ����� '/'
		sprintf(m_pszLogPath, "%s/", pszLogPath);
		m_pszLogPath[nLogPathLen+1] = '\0';
	}
	else
	{
		strcpy(m_pszLogPath, pszLogPath);
	}
	
	return PC_RESULT_SUCCESS;
}

int CPCLog::WriteLogFmt(const char* pFuncName, unsigned long ulLine, int nLevel, const char* pszFmt, ...)
{
	if (nLevel < eLevelClose || nLevel > eLevelFatal || pszFmt == NULL)
	{
		return PC_RESULT_PARAM;
	}
	CPCGuard guard(m_Mutex);

	//��־�ر���д
	if (eLevelClose == m_nLogLevel)
	{
		return PC_RESULT_SUCCESS;
	}
	//С���趨�ȼ�����־��д
	if (nLevel < m_nLogLevel)
	{
		return PC_RESULT_SUCCESS;
	}
	//���Logger��ͨ����д
	char pszTimeBuf[PC_MAX_PATH] = { 0 };
	int nRet = CheckLogger(pszTimeBuf);
	if (PC_RESULT_SUCCESS != nRet)
	{
		return nRet;
	}

	//д��־
	va_list ap;
	va_start(ap, pszFmt);
	nRet = vsprintf(m_pFmtBuff, pszFmt, ap);
    if(nRet < 0)
    {
        return PC_RESULT_SYSERROR;
    }
	va_end(ap);

	unsigned long ulThradID = PCGetCurrentThreadID();
	if (pFuncName == NULL || pFuncName[0] == 0 || ulLine == 0)
	{
		fprintf(m_pLogFile, "%s %s %lu %s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID,  m_pFmtBuff);
	}
	else
	{
		fprintf(m_pLogFile, "%s %s %lu %s:%lu %s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pFuncName, ulLine, m_pFmtBuff);
	}
	if(PC_LOG_WRITE_ALWAYS)
	{
		fflush(m_pLogFile);
	}
	
	//д������̨
	if (m_bStdout)
	{
		if (pFuncName == NULL || pFuncName[0] == 0 || ulLine == 0)
		{
			printf("%s %s %lu %s\r\n\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, m_pFmtBuff);
		}
		else
		{
			printf("%s %s %lu %s:%lu %s\r\n\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pFuncName, ulLine, m_pFmtBuff);
		}
	}
	return PC_RESULT_SUCCESS;
}

int CPCLog::WriteLogBytes(const char* pFuncName, unsigned long ulLine, int nLevel, const char* pszTips, const unsigned char* pszBytes, unsigned int nBytesLen)
{
	size_t nTipsLen = pszTips ? strlen(pszTips) : 0;
	if (nLevel < eLevelClose || nLevel > eLevelFatal || nBytesLen > (PC_LOG_LINE_MAX_LEN - nTipsLen - 64))
	{
		return PC_RESULT_PARAM;
	}
	CPCGuard guard(m_Mutex);

	//��־�ر���д
	if (eLevelClose == m_nLogLevel)
	{
		return PC_RESULT_SUCCESS;
	}
	//С���趨�ȼ�����־��д
	if (nLevel < m_nLogLevel)
	{
		return PC_RESULT_SUCCESS;
	}
	//���Logger��ͨ����д
	char pszTimeBuf[PC_MAX_PATH] = { 0 };
	int nRet = CheckLogger(pszTimeBuf);
	if (PC_RESULT_SUCCESS != nRet)
	{
		return nRet;
	}

	//���ֽ�����ת��Ϊ����ʾ���ַ���
	nRet = PCBytes2DispHexStr(pszBytes, nBytesLen, m_pFmtBuff, PC_LOG_LINE_MAX_LEN, " ");
	if (nRet < 0)
	{
		return nRet;
	}
	unsigned long ulThradID = PCGetCurrentThreadID();
	if (pFuncName == NULL || pFuncName[0] == 0 || ulLine == 0)
	{
		fprintf(m_pLogFile, "%s %s %lu %s%s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pszTips, m_pFmtBuff);
	}
	else
	{
		fprintf(m_pLogFile, "%s %s %lu %s:%lu %s%s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pFuncName, ulLine, pszTips, m_pFmtBuff);
	}
	if (PC_LOG_WRITE_ALWAYS)
	{
		fflush(m_pLogFile);
	}	

	//д������̨
	if (m_bStdout)
	{
		if (pFuncName == NULL || pFuncName[0] == 0 || ulLine == 0)
		{
			printf("%s %s %lu %s%s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pszTips, m_pFmtBuff);
		}
		else
		{
			printf("%s %s %lu %s:%lu %s%s\r\n", m_LOG_LEVEL_NAME[nLevel], pszTimeBuf, ulThradID, pFuncName, ulLine, pszTips, m_pFmtBuff);
		}
	}
	return PC_RESULT_SUCCESS;
}

int CPCLog::CheckLogger(char pszTimeBuf[PC_MAX_PATH] )
{
	if (pszTimeBuf == NULL)
	{
		return PC_RESULT_PARAM;
	}

	//��ȡʱ���ַ����Ƚϳ���
	int nTimeStrCmpLen = 0;
	if (eGenModeHour == m_nGenMode)
	{
		nTimeStrCmpLen = 13;
	}
	else if (eGenModeDay == m_nGenMode)
	{
		nTimeStrCmpLen = 10;
	}
	else
	{
		return PC_RESULT_SYSERROR;
	}

	//�����ǰ�ļ��͵�ǰʱ�䲻ͬ�����´򿪡�
	CPCTimeValue tv = CPCTimeValue::Now();
	int nRet = tv.Format("%Y_%m_%d_%H_%M_%S_@@@", pszTimeBuf, PC_MAX_PATH);
	if (nRet != PC_RESULT_SUCCESS)
	{
		return nRet;
	}
	if (m_pLogFile == NULL || 0 != memcmp(pszTimeBuf, m_pszCurrFileTime, nTimeStrCmpLen))
	{
		if (m_pLogFile != NULL)
		{
			fflush(m_pLogFile);
			fclose(m_pLogFile);
			m_pLogFile = NULL;
		}

		//��ȡ��־�ļ��е�ȫ·��
		char pszLogFolderFullPath[PC_MAX_PATH] = { 0 };
		if (eGenModeHour == m_nGenMode)
		{
			char pszFolderName[PC_MAX_PATH] = { 0 };
			memcpy(pszFolderName, pszTimeBuf, 10);
			sprintf(pszLogFolderFullPath, "%s%s/", m_pszLogPath, pszFolderName);
		}
		else
		{
			sprintf(pszLogFolderFullPath, "%s", m_pszLogPath);
		}

		//�ж���־�ļ���·���Ƿ����
		nRet = PCResExist(0, pszLogFolderFullPath);
		if (nRet < 0)
		{
			return nRet;
		}
		else if (nRet == 0)
		{
			//�������򴴽�
			nRet = PCMkdirRecursive(pszLogFolderFullPath);
			if (PC_RESULT_SUCCESS != nRet)
			{
				return nRet;
			}
		}

		//��ȡ��־�ļ���
		char pszLogFileName[PC_MAX_PATH] = { 0 };
		memcpy(m_pszCurrFileTime, pszTimeBuf, nTimeStrCmpLen);
		m_pszCurrFileTime[nTimeStrCmpLen] = '\0';
		sprintf(pszLogFileName, "%s%s.log", pszLogFolderFullPath, m_pszCurrFileTime);

		//���ļ�
		m_pLogFile =  fopen (pszLogFileName, "a+");
		if (NULL == m_pLogFile)
		{
			m_pLogFile = NULL;
			return PC_RESULT_FILEOPEN;
		}
	}

	return PC_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
