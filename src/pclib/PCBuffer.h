#ifndef _PCBUFFER_H_
#define _PCBUFFER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//CPCBuffer对象默认的初始化容量和空间增长的额外大小
#define PC_BUFFER_INIT_SIZE  (4096)

/**
*@brief		用户层缓冲区
*/
class CPCBuffer : CPCNoCopyable
{
public:
	//使用初始容量初始化一个buffer
    explicit CPCBuffer(size_t nInitSize = PC_BUFFER_INIT_SIZE)
		: m_Buffer(nInitSize)
		, m_Size(0)
	{ 
	}
	
	//使用初始字节数组初始化一个buffer
	explicit CPCBuffer(const char * data, size_t len)
		: m_Buffer(len)
		, m_Size(0)
	{ 
		Append(data,len); 
	}

	//使用初始字符串初始化一个buffer
    explicit CPCBuffer(const char * pstr)
		: m_Buffer(pstr ? strlen(pstr) : 0)
		, m_Size(0)
	{
		Append(pstr);
	}

    //将buffer重置成指定长度
	void Reset(size_t nResetSize = 0)
	{
        if(nResetSize > m_Size)
        {
            CheckBufIncrease(nResetSize - m_Size);
        }
		m_Size = nResetSize;
    }

	//在buffer后面添加一个字节数组
	void Append(const char*  data, size_t len)
	{
		if (!data)
		{
			PC_DEBUG_LOG("append NULL.");
			return;
		}
		CheckBufIncrease(len);
		
		std::copy(data, data + len, m_Buffer.begin() + m_Size);
		m_Size += len;
	}

	//在buffer后面添加一个字符串
	void Append(const char * pstr)
	{
		if (!pstr)
		{
			PC_DEBUG_LOG("append NULL.");
			return;
		}
		Append(pstr, strlen(pstr));
	}

	//获取buffer保存的字节数组及其长度
	const char* Buffer(size_t &nBufSize) const
	{
		nBufSize = m_Size;
		return &*m_Buffer.begin();
	}

	//获取buffer保存的以'\0'结束的字符串
	const char* C_Str()
	{
		CheckBufIncrease(1);
		m_Buffer[m_Size] = 0;
		return &*m_Buffer.begin();
	}
	
private:
	void CheckBufIncrease(size_t nIncSize)
	{
		if (m_Size + nIncSize > m_Buffer.capacity())
		{
            m_Buffer.resize(m_Buffer.capacity() + nIncSize + PC_BUFFER_INIT_SIZE);
		}
	}
	std::vector<char>	m_Buffer;
	size_t				m_Size;
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif /*_PCBUFFER_H_*/
