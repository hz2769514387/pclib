#if !defined(_PCTIMEVALUE_H_)
#define _PCTIMEVALUE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//��׼ʱ���(��ʽΪ 2014-08-12 12:08:00)�ַ����ĳ���
#define PC_TIMESTAMP_LEN		(19)

//�����ʽ������չ��ʾ�����ȱ���Ϊ3
#define PC_MS_FORMAT_STR		"@@@"

/**
*@brief		ʱ���࣬ͳһ�ṩʱ�䴦����
*/
class CPCTimeValue
{
public:
	//Ĭ�ϳ�ʼ��Ϊ0
	CPCTimeValue() :m_TimeMsValue(0){}
	virtual ~CPCTimeValue(){}

	//ͨ�������ʱ���ʼ��ʱ��ֵ��������ʽ���죬��Ҫ��Ϊ�˷�ֹ������ʱ���+-�ĺ���ʱ�����
	explicit CPCTimeValue(long long nTimeMsValue) :m_TimeMsValue(nTimeMsValue){}

	//����=(��ֵ)����������time_t��timeval��ʱ����ַ���(��:2016-05-08 12:34:56)ת��ΪCPCTimeValue
	CPCTimeValue& operator= (long long nTimeMsValue)	{ m_TimeMsValue = nTimeMsValue; return *this; }
	CPCTimeValue& operator= (timeval tvTime)			{ m_TimeMsValue = tvTime.tv_sec * 1000 + (tvTime.tv_usec / 1000); return *this; }
	CPCTimeValue& operator= (const char * pszTimeStamp);

	//���رȽϲ��������ж�����CPCTimeValue�Ĵ�С
	bool operator==(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue == rhs.m_TimeMsValue); }
	bool operator!=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue != rhs.m_TimeMsValue); }
	bool operator>(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue > rhs.m_TimeMsValue); }
	bool operator<(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue < rhs.m_TimeMsValue); }
	bool operator>=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue >= rhs.m_TimeMsValue); }
	bool operator<=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue <= rhs.m_TimeMsValue); }

	//����-(ʱ��)����������ʾ����ʱ��֮��ļ�������룩
	long long operator-(const CPCTimeValue& rhs) const	{return m_TimeMsValue - rhs.m_TimeMsValue;}

	//����+ -(����)����������ʾ�������ʱ�����ӻ���ٶ��ٺ��룬����һ���¶���
	const CPCTimeValue operator+(long long nMs) const	{return CPCTimeValue(m_TimeMsValue + nMs);}
	const CPCTimeValue operator-(long long nMs) const	{return CPCTimeValue(m_TimeMsValue - nMs);}

	//����+= -=(����)����������ʾ�ڵ�ǰ����Ļ����Ͻ�ʱ�����ӻ���ٶ��ٺ���
	CPCTimeValue& operator += (long long nMs)			{m_TimeMsValue += nMs;return *this;}
	CPCTimeValue& operator -= (long long nMs)			{m_TimeMsValue -= nMs;return *this;}

	/**
	*@brief		����C��׼��ʽ��ʱ�䣬��չPC_MS_FORMAT_STR�������,���ڶ��PC_MS_FORMAT_STRʱֻ�е�һ������Ч��
	*@param		pszFormatStr	[IN]	��ʽ������"%Y-%m-%d %H:%M:%S @@@"
    *@param		pszResult		[OUT]	��ʽ��������ݴ�Ż�����m_TimeMsValue
	*@param		nResultSize		[IN]	��ʽ��������ݴ�Ż���������
	*@return	�����룬��PC_Lib.h
	*/
	int Format(const char * pszFormatStr, char *pszResultBuf, unsigned int nResultBufSize) const;

    //�жϵ�ǰʱ���Ƿ��Ѿ�����this�������һ���趨��ʱ��nTimeoutMs���룿���nTimeoutMs < 0 ����������ʱ(���Ƿ���false).
    bool IsTimeOut(long long nTimeOutMs) const           {return ((nTimeOutMs < 0) ? false : (CPCTimeValue::Now().GetValue() - m_TimeMsValue > nTimeOutMs)); }

	//���ص�ǰϵͳ������ʱ�䣨��1970��1��1�������ĺ�������.
    static CPCTimeValue Now();

	//����ϵͳ���������ڵĺ�������TickCount.
    static CPCTimeValue TickCount();

	//��CPCTimeValue����ȡ������ʱ��
	long long GetValue(void) const		{ return m_TimeMsValue; }
private:
	//�ڲ���������ʱ�䣨��1970��1��1�������ĺ�������
	long long m_TimeMsValue;
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCTIMEVALUE_H_)
