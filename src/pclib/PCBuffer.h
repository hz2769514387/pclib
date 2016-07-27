#ifndef _PCBUFFER_H_
#define _PCBUFFER_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//CPCBuffer����Ĭ�ϵĳ�ʼ�������Ϳռ������Ķ����С
#define PC_BUFFER_INIT_SIZE  (4096)

/**
*@brief		�û��㻺����
*/
class CPCBuffer : CPCNoCopyable
{
public:
	//ʹ�ó�ʼ������ʼ��һ��buffer
    explicit CPCBuffer(size_t nInitSize = PC_BUFFER_INIT_SIZE)
		: m_Buffer(nInitSize)
		, m_Size(0)
	{ 
	}
	
	//ʹ�ó�ʼ�ֽ������ʼ��һ��buffer
	explicit CPCBuffer(const char * data, size_t len)
		: m_Buffer(len)
		, m_Size(0)
	{ 
		Append(data,len); 
	}

	//ʹ�ó�ʼ�ַ�����ʼ��һ��buffer
    explicit CPCBuffer(const char * pstr)
		: m_Buffer(pstr ? strlen(pstr) : 0)
		, m_Size(0)
	{
		Append(pstr);
	}

    //��buffer���ó�ָ������
	void Reset(size_t nResetSize = 0)
	{
        if(nResetSize > m_Size)
        {
            CheckBufIncrease(nResetSize - m_Size);
        }
		m_Size = nResetSize;
    }

	//��buffer�������һ���ֽ�����
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

	//��buffer�������һ���ַ���
	void Append(const char * pstr)
	{
		if (!pstr)
		{
			PC_DEBUG_LOG("append NULL.");
			return;
		}
		Append(pstr, strlen(pstr));
	}

	//��ȡbuffer������ֽ����鼰�䳤��
	const char* Buffer(size_t &nBufSize) const
	{
		nBufSize = m_Size;
		return &*m_Buffer.begin();
	}

	//��ȡbuffer�������'\0'�������ַ���
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
