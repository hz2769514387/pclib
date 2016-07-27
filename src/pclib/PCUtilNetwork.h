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


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
