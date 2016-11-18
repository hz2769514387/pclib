#ifndef _PC_TCPSOCKHANDLE_H_
#define _PC_TCPSOCKHANDLE_H_
#if defined(_MSC_VER)
# pragma once
#endif
#include "PCLock.h"
#include "PCBuffer.h"
#include "PCTcpPoller.h" 

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#define PER_SOCK_REQBUF_SIZE	(4096)	//ÿ���¼�Ͷ�ݵĻ������ݵĴ�С
#define MAX_IP_LEN				(128)	//IP��ַ�ĵ��ʮ�����ַ�����󳤶�

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
	//��������
	enum eSockType
	{
		eListenType = 0,	//�������ͣ�һ�����ڷ���˼����׽��֣�
		eAcceptType,		//�����������ͣ�һ�����ڷ���˴����׽��֣�
		eConnectType		//�����������ͣ�һ�����ڿͻ��ˣ�
	};
	CPCTcpSockHandle(eSockType eType);
	~CPCTcpSockHandle();

	//////////////////////////////////////////////////////////////////////////////
	// �ӿں��������û���������
	//<������������Ϊͬ���������ᴥ���麯��֪ͨ��Ͷ�ݺ�������ܻᴥ��֪ͨ>
	//////////////////////////////////////////////////////////////////////////////
	
	//������������������������������socket��nPort�ڣ�0~65535��֮�䣬�������Զ�������������ͻ���socket��nPort���Ϸ����� 
	bool Create(int nPort, bool bBlock = false);
	//��������bGracefully = true ʱ�����ŵعر����ӣ�����Ϊǿ�ƹر����ӡ�
	void Cleanup(bool bGracefully = false);
	
	//Ͷ������һ�����û��������ã�Ͷ�ݷ�������֮ǰ����Ҫ���н����ݷ���m_SendBuffer
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
	eSockType	m_SocketType;				//SOCKET����
	PC_SOCKET	m_SocketFd;					//�ڲ�ά����һ��SOCKET
	PC_SOCKET	m_ListenSocketFd;			//����Accept���͵�socket���ڲ�ά������socket��������
	char		m_pszRemoteIP[MAX_IP_LEN];	//�Է���IP��ַ(�����accept��socket)
	CPCBuffer	m_SendBuffer;				//���η��ͻ�����
	CPCBuffer	m_RecvBuffer;				//���ν��ջ�����

#if defined (_WIN32)
	//������
	enum eOpt
	{
		eUnconnect = 0,
		eConnect,
		eAccept,
		eRead,
		eWrite
	};
	eOpt			m_Opt;					//������
	IOCP_IO_CTX		m_ioCtx;				//ԭʼ�ص��ṹ
	WSABUF			m_wsBufPointer;			//Ͷ������BUFָ��
#else
    //����Linux���ڲ���Ҫά��CPCTcpPollerThreadָ��
	CPCTcpPollerThread* m_pPollerThread ;	//epoll_wait�߳�
	int				m_events;				//Ҫ�ύ���¼���������������ϣ�EPOLLIN EPOLLOUT EPOLLPRI EPOLLERR EPOLLHUP EPOLLET EPOLLONESHOT
	int				m_epctlOp;				//Ҫ���¼����еĶ����������ǣ�EPOLL_CTL_ADD EPOLL_CTL_MOD EPOLL_CTL_DEL
#endif

private:
	unsigned int	m_ActualSendedLen;		//ʵ�ʷ��͵ĳ���
};

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	/*_PC_TCPSOCKHANDLE_H_*/
