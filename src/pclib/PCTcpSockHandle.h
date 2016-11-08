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
*@brief		�ض�ƽ̨�첽ģ�͵Ľṹ����
*/
#if defined (_WIN32)

//ÿ����IOCPͶ����������ݽṹ��������Ҫ��Ϊ�˱���AcceptExʱ�Ĵ���ָ��
class CPCTcpSockHandle;
typedef struct _IOCP_IO_CTX
{
	OVERLAPPED		m_olOriginal;	//ԭʼ�ص��ṹ
	CPCTcpSockHandle*	m_pOwner;	//���Ͷ�ݵ����������ڵ�����
} IOCP_IO_CTX;

#else
#endif

/**
*@brief		TCP�����࣬�ڲ�ά��һ��Socket
*			������ӿ����Ƿ���˼����������Ϳͻ����������͵�����
*			Ϊ�˿�ƽ̨�ͱ����ڴ�й©����������ཫ�������ݺͲ����󶨣���ÿ��CPCTcpSockHandle����
*				����״̬�ģ����ض���ĳ��ʱ��ֻ������һ��״̬���޷��ص����������������windowsƽ̨��
*				��һ����������ʧ����
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

	//�ص���Ľ�һ��������ҪĿ����Ϊ�����β���ϵͳ����
	void ProcessAccept();
	void ProcessConnect();
	void ProcessSend(unsigned long dwSendedLen);
	void ProcessRecv(unsigned long dwRecvedLen);
	void ProcessClose();
	
	//��������Ľ�һ����������ʵ����Щ��������ҵ���߼�
	virtual void OnAccepted(){}
	virtual void OnConnected(){}
	virtual void OnSendded(unsigned long dwSendedLen){}
	virtual void OnRecved(unsigned long dwRecvedLen){}
	virtual void OnClosed(){}

public:
	PC_SOCKET	m_hTcpSocket;					//�ڲ�ά����һ��SOCKET
	bool		m_bListenSocket;				//SOCKET�����Ƿ�Ϊ��������
	char		m_pszRemoteIP[MAX_IP_STR_LEN];	//�Է���IP��ַ(�����accept��socket)
	CPCRecursiveLock	m_Mutex;				//�������ṩ����

	

#if defined (_WIN32)
	//������
	enum csEnumOpt
	{
		cseUnconnect = 0,
		cseConnect,
		cseAccept,
		cseRead,
		cseWrite
	};
	csEnumOpt		m_ConnOpt;						//������
	IOCP_IO_CTX		m_ioCtx;						//ԭʼ�ص��ṹ
	char			m_szIOBuf[PER_SOCK_REQBUF_SIZE];//���ͻ���յ����ݻ�����
	unsigned long	m_dwIOBufLen;					//���ݳ���(����ʱ��ָ�����ȣ�����ʱ��ֵ���ǽ������ݵĻ���������)
	WSABUF			m_wsBufPointer;					//Ͷ������BUFָ��
#else
    //����Linux���ڲ���Ҫά��epoll��صĵľ��
    int	m_epollFd;
    int m_eventFd;
#endif
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
