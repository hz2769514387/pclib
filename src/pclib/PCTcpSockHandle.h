#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"
#include "PCBuffer.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//ÿ���¼�Ͷ�ݵĻ������ݵĴ�С
#define MAX_IP_LEN				(128)	//IP��ַ�ĵ��ʮ�����ַ�����󳤶�


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
*@brief		TCP�����࣬�ڲ�ά��һ��Socket��������ӿ����Ƿ���˼����������Ϳͻ����������͵�����
*			Ϊ�˿�ƽ̨�ͱ����ڴ�й©����������ཫ�������ݺͲ����󶨣���ÿ��CPCTcpSockHandle����
*				����״̬�ģ����ض���ĳ��ʱ��ֻ������һ��״̬���޷��ص����������������windowsƽ̨����һ����������ʧ����
*			����ܹ����Ϊ���㣺�Ե���������Ϊ��
*				1.�����¼��ַ��㣬����ײ�������¼�Դ����CPCTcpPoller��CPCTcpPollerThread��ͬ��ɣ�������buffer���û�ֻ��Ҫ�����͹ر����������������
*				2.�����¼�����㣬��ɳ���ʱ��Ĵ󲿷������¼��Ĵ���ͬʱĨƽ����ϵͳ���졣��CPCTcpSockHandle��ɣ��ֱ������buffer�ͽ���buffer
*				3.�������ݴ���㣬�����ϲ�ҵ���߼������û��Լ��̳�CPCTcpSockHandle�ಢʵ�����麯����ɣ��û�������ȫ���õڶ���ķ���buffer�������Խ���Ҫ
*					Ͷ�ݵ����ݴ�����ڶ���ķ���buffer�У�Ȼ��Ͷ�ݷ������󣩣�������ȫ�����ڶ���Ľ���buffer���ֶ�ν���ʱ��Ҫ�Լ�����ÿ�ν��յĽ����
*/
class CPCTcpSockHandle : CPCNoCopyable
{
public:
	CPCTcpSockHandle();
	~CPCTcpSockHandle();

	//////////////////////////////////////////////////////////////////////////////
	// �ӿں��������û���������
	//////////////////////////////////////////////////////////////////////////////
	//������������������������������socket��nPort�ڣ�0~65535��֮�䣬�������Զ�������������ͻ���socket��nPort���Ϸ����� 
	bool Create(int nPort, bool bBlock = false);
	//��������bGracefully = true ʱ�����ŵعر����ӣ�����Ϊǿ�ƹر����ӡ�
	void Cleanup(bool bGracefully = false);
	//Ͷ������һ�����û���������<Ͷ�ݷ�������ʱ��������󳤶�Ϊ PER_SOCK_REQBUF_SIZE>
	bool PostConnect(const char *pszHostAddress, int nPort);
	bool PostSend();
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//////////////////////////////////////////////////////////////////////////////
	// �Ѿ�ʵ�ֵĻص��������ص���Ľ�һ��������ҪĿ����Ϊ�����β���ϵͳ���죬�û���Ҫ����
	//////////////////////////////////////////////////////////////////////////////
	void ProcessAccept();
	void ProcessConnect();
	void ProcessSend(unsigned long dwSendedLen);
	void ProcessRecv(unsigned long dwRecvedLen);
	void ProcessClose();
	
	//////////////////////////////////////////////////////////////////////////////
	// δʵ�ֵĻص���������������Ľ�һ���������û��̳д����ʵ����Щ��������ҵ���߼�
	//////////////////////////////////////////////////////////////////////////////
	virtual void OnAccepted(){}
	virtual void OnConnected(){}
	virtual void OnSendded(){}
	virtual void OnRecved(unsigned long dwRecvedLen){}
	virtual void OnClosed(){}

public:
	CPCRecursiveLock	m_Mutex;			//�������ṩ����
	PC_SOCKET	m_hTcpSocket;				//�ڲ�ά����һ��SOCKET
	bool		m_bListenSocket;			//SOCKET�����Ƿ�Ϊ��������
	char		m_pszRemoteIP[MAX_IP_LEN];	//�Է���IP��ַ(�����accept��socket)
	CPCBuffer	m_SendBuffer;				//���η��ͻ�����
	CPCBuffer	m_RecvBuffer;				//���ν��ջ�����

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
	csEnumOpt		m_ConnOpt;				//������
	IOCP_IO_CTX		m_ioCtx;				//ԭʼ�ص��ṹ
	WSABUF			m_wsBufPointer;			//Ͷ������BUFָ��
#else
    //����Linux���ڲ���Ҫά��epoll��صĵľ��
    int	m_epollFd;
    int m_eventFd;
#endif

private:
	unsigned int	m_ActualSendedLen;		//ʵ�ʷ��͵ĳ���
};


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
