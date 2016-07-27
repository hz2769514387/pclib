#ifndef _PCTHREADBASE_H__
#define _PCTHREADBASE_H__
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCMemory.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

//�߳�ջ��С����
#define PC_MIN_THREAD_STACK_SIZE	(16384)

/**
*@brief		�߳���
*			
*/
class CPCThread :CPCNoCopyable
{
public:
	explicit CPCThread();
	virtual ~CPCThread(){ StopThread(); }
	
	//�̵߳Ĵ���������ֹͣ��ֻ���߳���ȫ��������ȫֹͣ�ŷ��ء���������õȴ����ٺ����ʱ����(<0����һֱ�ȴ�)
	bool	StartThread(int nTimeoutMs = -1);	
    void	StopThread(int nTimeoutMs = -1);

    //�̵߳���ڣ�������õ��Ǽ̳У�����Ҫ���Ǵ˺�����
    virtual void Svc() = 0;

	//�����ú���
	void SetStackSize(unsigned int nStackSize);						//�����߳�ջ��С��������StartThread֮ǰ���ò���Ч���û�һ���������
	void SetRunning(bool bRunning)	{ m_bRunning = bRunning; }		//m_bRunning���ã��û�һ���������
	void SetExited(bool bExited)	{ m_bThreadExited = bExited; }	//m_bThreadExited���ã��û�һ���������
	
protected:
	
	unsigned int		m_nStackSize;		//�߳�ջ��С
	CPCRecursiveLock	m_Mutex;			//�������ṩ����

	bool				m_bRunning;			//�߳��Ƿ��������У��˱���һ������ѭ�����������߳��ж����߳��Ƿ�����
	bool				m_bThreadExited;	//�߳��Ƿ��Ѿ��˳��ˣ�һ���������̵߳ȴ����߳��˳�
	bool				m_bIsCreated;		//�߳��Ƿ��Ѿ�����
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif/*_THREADBASE_H__*/
