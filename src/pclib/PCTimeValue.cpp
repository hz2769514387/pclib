#include "PC_Lib.h"
#include "PCTimeValue.h" 
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


CPCTimeValue& CPCTimeValue::operator= (const char * pszTimeStamp)
{
	if (pszTimeStamp == NULL || strlen(pszTimeStamp) != PC_TIMESTAMP_LEN)
	{
		PC_ASSERT(true, "param error!pszTimeStmp=%s", pszTimeStamp);
		return *this;
	}

	//����ԭ����tm�ṹ��
	struct tm tmTimeMsValue;
	char szYear[5] = { 0 }, szMonth[3] = { 0 }, szDay[3] = { 0 }, szHour[3] = { 0 }, szMinute[3] = { 0 }, szSecond[3] = { 0 };
	memcpy(szYear, pszTimeStamp, 4);
	memcpy(szMonth, pszTimeStamp + 5, 2);
	memcpy(szDay, pszTimeStamp + 8, 2);
	memcpy(szHour, pszTimeStamp + 11, 2);
	memcpy(szMinute, pszTimeStamp + 14, 2);
	memcpy(szSecond, pszTimeStamp + 17, 2);
	tmTimeMsValue.tm_year = atoi(szYear) - 1900;
	tmTimeMsValue.tm_mon = atoi(szMonth) - 1;
	tmTimeMsValue.tm_mday = atoi(szDay);
	tmTimeMsValue.tm_hour = atoi(szHour);
	tmTimeMsValue.tm_min = atoi(szMinute);
	tmTimeMsValue.tm_sec = atoi(szSecond);

	//tmת��Ϊ����
	time_t tMakeTime = mktime(&tmTimeMsValue) * 1000;
	if (tMakeTime < 0)
	{
		PC_ASSERT(true, "mktime error!pszTimeStmp=%s", pszTimeStamp);
		return *this;
	}

	m_TimeMsValue = tMakeTime;
	return *this;
}


int CPCTimeValue::Format(const char * pszFormatStr, char *pszResultBuf, unsigned int nResultBufSize) const
{
	if (pszFormatStr == NULL || pszFormatStr[0] == 0 || pszResultBuf == NULL || nResultBufSize == 0 || nResultBufSize < (strlen(pszFormatStr) + 10))
	{
		return PC_RESULT_PARAM;
	}

	//�������ͺ���
	time_t nSec = m_TimeMsValue / 1000;
	int nMs = m_TimeMsValue % 1000;

	//��time_tת��Ϊtm�ṹ��
	struct tm tmTimeMsValue;
#if defined (_WIN32)
	if (0 != localtime_s(&tmTimeMsValue, &nSec))
	{
		return PC_RESULT_SYSERROR;
	}
#else
	if (NULL == localtime_r(&nSec, &tmTimeMsValue))
	{
		return PC_RESULT_SYSERROR;
	}
#endif

	//����C��׼��ʽ��ʱ��
	strftime(pszResultBuf, nResultBufSize, pszFormatStr, &tmTimeMsValue);

	//�������
	char * pMsPos = strstr(pszResultBuf, PC_MS_FORMAT_STR);
	if (NULL == pMsPos)
	{
		return PC_RESULT_SUCCESS;
	}
	pMsPos[0] = '0' + nMs / 100;
	pMsPos[1] = '0' + nMs / 10 % 10;
	pMsPos[2] = '0' + nMs % 10;

	return PC_RESULT_SUCCESS;
}

CPCTimeValue CPCTimeValue::Now()
{
#if defined (_WIN32)
	struct timeb tbTime;
	ftime(&tbTime);
	return CPCTimeValue(tbTime.time * 1000 + tbTime.millitm);
#else
	/* ��Ҫ���ϱ���ѡ�� -lrt */
	struct timespec ts;
    int nRet = clock_gettime(CLOCK_REALTIME, &ts);
    PC_ASSERT(nRet == 0, "clock_gettime fail!");
	return CPCTimeValue((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
#endif
}

CPCTimeValue CPCTimeValue::TickCount()
{
#if defined(_WIN32)
	static LARGE_INTEGER TicksPerSecond = { 0 };
	if (!TicksPerSecond.QuadPart)
	{
		QueryPerformanceFrequency(&TicksPerSecond);
	}
	LARGE_INTEGER Tick;
	QueryPerformanceCounter(&Tick);

	long long Seconds = Tick.QuadPart / TicksPerSecond.QuadPart;
	long long LeftPart = Tick.QuadPart - (TicksPerSecond.QuadPart*Seconds);
	long long MillSeconds = LeftPart * 1000 / TicksPerSecond.QuadPart;
	long long nRet = Seconds * 1000 + MillSeconds;
	
	PC_ASSERT(nRet > 0, "nRet <= 0");
	return CPCTimeValue(nRet);
#else
	/* ��Ҫ���ϱ���ѡ�� -lrt */
	struct timespec ts;
    int nRet = clock_gettime(CLOCK_MONOTONIC, &ts);
    PC_ASSERT(nRet == 0, "clock_gettime fail!");
	return CPCTimeValue((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
#endif
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
