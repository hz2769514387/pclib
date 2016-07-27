#ifndef _PCBLOCKINGQUEUE_H_
#define _PCBLOCKINGQUEUE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


/**
*@brief		JAVA÷–µƒ◊Ë»˚∂”¡–(BlockingQueue¿‡)°£
*/
template <typename T> class CPCBlockingQueue :CPCNoCopyable
{
public:

	CPCBlockingQueue(void)
		: m_Mutex()
		, m_NotEmpty(m_Mutex)
		, m_UntilEmpty(m_Mutex)
	{}

	void Put(const T& v)
	{
		CPCGuard guard(m_Mutex);

		m_Deque.push_back(std::move(v));
		m_NotEmpty.Notify();
	}

	T Take(void)
	{
		CPCGuard guard(m_Mutex);

		// ±‹√‚–ÈºŸªΩ–—
		while (m_Deque.empty())
		{
			m_NotEmpty.Wait();
		}
		PC_LOG_ASSERT(!m_Deque.empty(),"queue is empty");
		
		T TFront(std::move(m_Deque.front()));

		m_Deque.pop_front();
		if (m_Deque.empty())
		{
			m_UntilEmpty.Notify();
		}
			
		return std::move(TFront);
	}

	T Front(void)
	{
		CPCGuard guard(m_Mutex);
		return (std::move(m_Deque.front()));
	}

	bool WaitEmpty(int tm_ms = -1)
	{
		CPCGuard guard(m_Mutex);

		// ±‹√‚–ÈºŸªΩ–—
		while (!m_Deque.empty()) 
		{
			if (!m_UntilEmpty.Wait(tm_ms))
			{
				return false;
			}
		}
		return true;
	}

	size_t Size(void) const
	{
		CPCGuard guard(m_Mutex);
		return m_Deque.size();
	}

	bool Empty(void) const
	{
		CPCGuard guard(m_Mutex);
		return m_Deque.empty();
	}
	
private:
	CPCRecursiveLock	m_Mutex;
	CPCCondition		m_NotEmpty;
	CPCCondition		m_UntilEmpty;
	std::deque<T>		m_Deque;
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif /*_PCBLOCKINGQUEUE_H_*/
