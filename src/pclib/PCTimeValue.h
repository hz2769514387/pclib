#if !defined(_PCTIMEVALUE_H_)
#define _PCTIMEVALUE_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//标准时间戳(形式为 2014-08-12 12:08:00)字符串的长度
#define PC_TIMESTAMP_LEN		(19)

//毫秒格式化的扩展表示，长度必须为3
#define PC_MS_FORMAT_STR		"@@@"

/**
*@brief		时间类，统一提供时间处理功能
*/
class CPCTimeValue
{
public:
	//默认初始化为0
	CPCTimeValue() :m_TimeMsValue(0){}
	virtual ~CPCTimeValue(){}

	//通过传入的时间初始化时间值，必须显式构造，主要是为了防止将日历时间和+-的毫秒时间混淆
	explicit CPCTimeValue(long long nTimeMsValue) :m_TimeMsValue(nTimeMsValue){}

	//重载=(赋值)操作符，将time_t、timeval、时间戳字符串(例:2016-05-08 12:34:56)转换为CPCTimeValue
	CPCTimeValue& operator= (long long nTimeMsValue)	{ m_TimeMsValue = nTimeMsValue; return *this; }
	CPCTimeValue& operator= (timeval tvTime)			{ m_TimeMsValue = tvTime.tv_sec * 1000 + (tvTime.tv_usec / 1000); return *this; }
	CPCTimeValue& operator= (const char * pszTimeStamp);

	//重载比较操作符，判断两个CPCTimeValue的大小
	bool operator==(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue == rhs.m_TimeMsValue); }
	bool operator!=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue != rhs.m_TimeMsValue); }
	bool operator>(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue > rhs.m_TimeMsValue); }
	bool operator<(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue < rhs.m_TimeMsValue); }
	bool operator>=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue >= rhs.m_TimeMsValue); }
	bool operator<=(const CPCTimeValue& rhs) const		{ return (m_TimeMsValue <= rhs.m_TimeMsValue); }

	//重载-(时间)操作符，表示两个时间之间的间隔（毫秒）
	long long operator-(const CPCTimeValue& rhs) const	{return m_TimeMsValue - rhs.m_TimeMsValue;}

	//重载+ -(毫秒)操作符，表示将对象的时间增加或减少多少毫秒，返回一个新对象
	const CPCTimeValue operator+(long long nMs) const	{return CPCTimeValue(m_TimeMsValue + nMs);}
	const CPCTimeValue operator-(long long nMs) const	{return CPCTimeValue(m_TimeMsValue - nMs);}

	//重载+= -=(毫秒)操作符，表示在当前对象的基础上将时间增加或减少多少毫秒
	CPCTimeValue& operator += (long long nMs)			{m_TimeMsValue += nMs;return *this;}
	CPCTimeValue& operator -= (long long nMs)			{m_TimeMsValue -= nMs;return *this;}

	/**
	*@brief		按照C标准格式化时间，扩展PC_MS_FORMAT_STR代表毫秒,存在多个PC_MS_FORMAT_STR时只有第一个才有效。
	*@param		pszFormatStr	[IN]	格式串，如"%Y-%m-%d %H:%M:%S @@@"
    *@param		pszResult		[OUT]	格式化后的数据存放缓冲区m_TimeMsValue
	*@param		nResultSize		[IN]	格式化后的数据存放缓冲区长度
	*@return	错误码，见PC_Lib.h
	*/
	int Format(const char * pszFormatStr, char *pszResultBuf, unsigned int nResultBufSize) const;

    //判断当前时间是否已经超过this对象最后一次设定的时间nTimeoutMs毫秒？如果nTimeoutMs < 0 代表永不超时(总是返回false).
    bool IsTimeOut(long long nTimeOutMs) const           {return ((nTimeOutMs < 0) ? false : (CPCTimeValue::Now().GetValue() - m_TimeMsValue > nTimeOutMs)); }

	//返回当前系统的日历时间（从1970年1月1日以来的毫秒数）.
    static CPCTimeValue Now();

	//返回系统启动到现在的毫秒数即TickCount.
    static CPCTimeValue TickCount();

	//从CPCTimeValue类中取出日历时间
	long long GetValue(void) const		{ return m_TimeMsValue; }
private:
	//内部保存日历时间（从1970年1月1日以来的毫秒数）
	long long m_TimeMsValue;
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCTIMEVALUE_H_)
