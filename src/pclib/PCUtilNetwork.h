#if !defined(_PCUTILNETWORK_H_)
#define _PCUTILNETWORK_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		����SOCKET����Ϊ������ģʽ
*@param		sock	[IN]	�׽���
*@return	�ɹ�Ϊ0��ʧ��Ϊ�����룬��PC_Lib.h
*/
int  PCSetNonBlocking(PC_SOCKET sock);

/**
*@brief		�ж�SOCKET�����Ƿ���������
*@param		sock	[IN]	�׽���
*@return	�ɹ�������Ϊ0��ʧ��Ϊ�����룬��PC_Lib.h
*/
int  PCSockIsConnected(PC_SOCKET sock);

/**
*@brief		�ر�һ�������ӵ�SOCKET
*@param		sock	[IN]	�׽���
*@return
*/
void PCShutdownSocket(PC_SOCKET sock);

/**
*@brief		����һ��Tcp�׽���
*@param		nPort	[IN]	�׽�����Ҫ�󶨵Ķ˿ڣ�һ����˵��ֻ�м����׽��ֲ���Ҫָ���˿ںţ���������-1����
*			blocked	[IN]	�Ƿ�������true���� false������
*@return	�ɹ��򷵻��׽�����������ʧ�ܷ���PC_INVALID_SOCKET
*/
PC_SOCKET  PCCreateTcpSocket(int nPort, bool blocked);

/**
*@brief		����һ��IPv4��ַ������ж����ֻ��ʹ�õ�һ��������ģʽ
*@param		pszHost		[IN]	һ�����ʮ���Ƶ�IPv4��ַ������
*@param		nPort		[IN]	ʮ���ƶ˿ں�
*@param		addr4		[OUT]	���������ĵ�ַ
*@return	�ɹ�Ϊ0��ʧ��Ϊ�����룬��PC_Lib.h
*/
int PCDnsParseAddrIPv4(const char *pszHost, int nPort, struct sockaddr_in *addr4);




//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
