#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//ÿ���¼�Ͷ�ݵĻ������ݵĴ�С���˲������ݻ������軻���ںˣ�Ҫ���ƴ�С
#define MAX_IP_STR_LEN			(128)	//IP��ַ�ĵ��ʮ�����ַ�����󳤶�

/**
*@brief		TCP�����࣬�ڲ�ά��һ��Socket
*			������ӿ����Ƿ���˼����������Ϳͻ����������͵�����
*/
class CPCTcpSockHandle : CPCNoCopyable
{
public:
	CPCTcpSockHandle();
	~CPCTcpSockHandle();

	//�������������Ӻ�����Create��������������������socket��nPort�ڣ�0~65535��֮�䣬�������Զ�������������ͻ���socket��nPort���Ϸ����� 
	bool Create(int nPort, bool bBlock = false);
	void Cleanup();

	//Ͷ������һ�����û���������<Ͷ�ݷ�������ʱ��������󳤶�Ϊ PER_SOCK_REQBUF_SIZE>
	bool PostConnect(const char *pszHostAddress, int nPort);
	bool PostSend(const char *szSendBuff, unsigned long nSendLen);
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//��������ص�����������ʵ�ּ��ɣ�����Ҫ�û���������(�����ʵ�ֽ��������ԣ�ʵ��ʹ��ʱ��Ҫ��д���า����Щ����)
	virtual void DoConnected(bool bSucceed){ PC_TRACE_LOG("connected"); }
	virtual void DoSendded(bool bSucceed, unsigned long dwSendedLen){ PC_TRACE_LOG("send(%lu) bytes", dwSendedLen); }
	virtual void DoRecved(bool bSucceed, const char *szRecvedBuff, unsigned long dwRecvedLen){ PC_TRACE_LOG("recv(%lu) bytes:%s", dwRecvedLen, szRecvedBuff); }
	virtual void DoClose(){ PC_TRACE_LOG("closed."); }

	//DoAccept�ṩ��Ĭ��ʵ�֡���������socket��һ�����ڷ��������յ��ͻ��˵�socket��
	virtual bool DoAccept(bool bSucceed, const char *szRecvedBuff, unsigned long dwRecvedLen);

public:
	PC_SOCKET	m_hTcpSocket;				//�ڲ�ά����һ��SOCKET
	bool		m_bListenSocket;			//SOCKET�����Ƿ�Ϊ��������
	char	m_pszRemoteIP[MAX_IP_STR_LEN];	//�Է���IP��ַ(�����accept��socket)
	CPCRecursiveLock	m_Mutex;			//�������ṩ����

#if defined (_WIN32)
#else
    //����Linux���ڲ���Ҫά��epoll��صĵľ��
    int	m_epollFd;
    int m_eventFd;
#endif
};


/**
*@brief		�ض�ƽ̨�첽ģ�͵Ľṹ����
*/
#if defined (_WIN32)

	//ÿ����IOCPͶ����������ݽṹ����Ϊ����Ҫʹ��ͬʱͶ�ݶ�εĹ��ܣ�������Ҫ�����д�Ͷ�ݵ������绺������ȫ������˽ṹ��
	typedef struct _IOCP_IO_CTX
	{
		OVERLAPPED		m_olOriginal;	//ԭʼ�ص��ṹ
		CPCTcpSockHandle*	m_pOwner;	//���Ͷ�ݵ����������ڵ�����
		BYTE			m_byOpType;		//��������

		char			m_szIOBuf[PER_SOCK_REQBUF_SIZE];//���ͻ���յ����ݻ�����
		unsigned long	m_dwIOBufLen;					//���ݳ���(����ʱ��ָ�����ȣ�����ʱ��ֵ���ǽ������ݵĻ���������)
		WSABUF			m_wsBufPointer;					//Ͷ������BUFָ��

		// ��ʼ���˽ṹ
		_IOCP_IO_CTX(BYTE byOpType, CPCTcpSockHandle* pOwner, const char* szIOBuf = NULL, unsigned long dwIOBufLen = PER_SOCK_REQBUF_SIZE)
		{
			ZeroMemory(&m_olOriginal, sizeof(m_olOriginal));
			m_pOwner = pOwner;
			m_byOpType = byOpType;
			ZeroMemory(m_szIOBuf, sizeof(m_szIOBuf));
			m_dwIOBufLen = sizeof(m_szIOBuf);
			if (szIOBuf)
			{
				if (dwIOBufLen <= PER_SOCK_REQBUF_SIZE)
				{
					memcpy(m_szIOBuf, szIOBuf, dwIOBufLen);
					m_dwIOBufLen = dwIOBufLen;
				}
				else
				{
					PC_WARN_LOG("dwIOBufLen(%ld) > %d, only copy(%d) length.", dwIOBufLen, PER_SOCK_REQBUF_SIZE, PER_SOCK_REQBUF_SIZE);
					memcpy(m_szIOBuf, szIOBuf, PER_SOCK_REQBUF_SIZE);
					m_dwIOBufLen = PER_SOCK_REQBUF_SIZE;
				}
			}
			m_wsBufPointer.buf = m_szIOBuf;
			m_wsBufPointer.len = m_dwIOBufLen;
		}

	} IOCP_IO_CTX;

	//�������ͣ���Ӧ_IOCP_IO_CTX.m_byOpType
	#define OP_UNKNOWN		0xFF
	#define OP_READ			0
	#define OP_WRITE		1
	#define OP_CONNECT		2
	#define OP_ACCEPT		3
#else

#endif

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
