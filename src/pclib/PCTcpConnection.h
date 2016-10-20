#ifndef _PC_TCPCONNECTION_H_
#define _PC_TCPCONNECTION_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define MAX_SOCK_CACHE_SIZE		(4096)	//�����໺�����ݵĴ�С���˲������ݻ������軻���ںˣ�Ҫ���ƴ�С
#define MAX_IP_STR_LEN			(128)	//IP��ַ�ĵ��ʮ�����ַ�����󳤶�

class CTcpConnection : CPCNoCopyable
{
public:
	CTcpConnection();
	~CTcpConnection();

	//�������������Ӻ�����Create��������������������socket��service��Ϊ�գ��������Զ�������������ͻ���socket��serviceΪ�գ��� 
	bool Create(const char * service, bool bBlock = false);
	void Cleanup();

	//Ͷ������һ�����û���������
	bool PostConnect(const char *pszHostAddress, const char * service);
	bool PostSend();
	bool PostRecv();
	bool PostAccept(PC_SOCKET sListen);

	//��������ص�����������ʵ�ּ��ɣ�����Ҫ�û���������
	virtual void DoConnected(){}
	virtual void DoSendded(unsigned long dwSize){}
	virtual void DoRecved(unsigned long dwSize){}
	virtual void DoClose(){}

	//DoAccept�ṩ��Ĭ��ʵ�֡���������socket��һ�����ڷ��������յ��ͻ��˵�socket��
	virtual bool DoAccept(unsigned long dwSize);

public:
	char m_szSendBuff[MAX_SOCK_CACHE_SIZE];	//�����͵����ݻ�����
	unsigned long m_dwSendLen;				//�����͵����ݳ���
	char m_szRecvBuff[MAX_SOCK_CACHE_SIZE];	//���ջ�����

	PC_SOCKET	m_hTcpSocket;				//�ڲ�ά����һ��SOCKET
	bool		m_bListenSocket;			//SOCKET�����Ƿ�Ϊ��������
	char	m_pszRemoteIP[MAX_IP_STR_LEN];	//�Է���IP��ַ

protected:
	//OVERLAPPEDPLUS		m_overLapped;		//���պ����ӵ��ص��ṹ
	//OVERLAPPEDPLUS		m_sendOverLapped;	//���͵��ص��ṹ	
	WSABUF				m_wsBuff;			//���պ����ӵ�BUFָ��
	WSABUF				m_wsSendBuff;		//���͵�BUFָ��
	LPFN_CONNECTEX		m_lpfnConnectEx;	//WSA��չ����CONNECT
	CPCRecursiveLock	m_Mutex;			//�������ṩ����
};



//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPCONNECTION_H_*/
