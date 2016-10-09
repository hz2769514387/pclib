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
*@brief		����һ��ָ���ͻ��˻��������͵�Tcp�׽���
*@param		service	[IN]	��serviceΪNULL��""����׽���Ϊ�ͻ����׽���
*							��serviceΪʮ���ƶ˿ںŻ������(http,ftp..etc)���׽���Ϊ������׽��֣�������ִ�м�������
*			blocked	[IN]	�Ƿ�������true���� false������
*@return	�ɹ��򷵻��׽�����������ʧ�ܷ���PC_INVALID_SOCKET
*/
PC_SOCKET  PCCreateTcpSocket(const char * service, bool blocked);

/**
*@brief		����һ��IPv4��ַ������ж����ֻ��ʹ�õ�һ��������ģʽ
*@param		hostname4	[IN]	һ�����ʮ���Ƶ�IPv4��ַ������
*@param		service		[IN]	ʮ���ƶ˿ںŻ������(http,ftp..etc)
*@param		addr4		[OUT]	���������ĵ�ַ
*@return	�ɹ�Ϊ0��ʧ��Ϊ�����룬��PC_Lib.h
*/
int PCDnsParseAddrIPv4(const char *hostname4, const char * service, struct sockaddr_in *addr4);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
