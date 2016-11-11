#if !defined(_PCSYSTEMUTIL_H_)
#define _PCSYSTEMUTIL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		��ȡ���Ĵ���lasterr
*@param		bNetWorkError	[IN]	�Ƿ�Ϊ��������룬0���� ��������
*@return	���Ĵ�����lasterrno
*/
int  PCGetLastError(bool bNetWorkError = false);

/**
*@brief		����ϵͳ������õ���������
*@param		nErrorNo		[IN]	ϵͳ������
*@param		pszErrMsgBuf	[OUT]	����������ŵĻ�����
*@param		nErrMsgBufLen	[IN]	����������ŵĻ������ĳ���
*@return	�����룬��PC_Lib.h
*/
int  PCGetSysErrorMsg(int nErrorNo, char* pszErrMsgBuf, unsigned int nErrMsgBufLen);

/**
*@brief		��һ��ȫ·�������Ŀ¼�����ļ������罫"c:\a\1.txt"�����"c:\a\"��1.txt
*@param		pszSrcFullPath	[IN]	����İ����ļ�����ȫ·����·���ָ���֧��\��/
*@param		pszPathBuf		[OUT]	Ŀ¼����ŵĻ���������󳤶�Ϊ PC_MAX_PATH���������NULL���������Ŀ¼��
*@param		pszPathBuf		[OUT]	�ļ�����ŵĻ���������󳤶�Ϊ PC_MAX_PATH���������NULL����������ļ���
*@return	�����룬��PC_Lib.h
*/
int  PCSeperatePath(const char * pszSrcFullPath, char pszPathBuf[PC_MAX_PATH], char pszFileName[PC_MAX_PATH]);

/**
*@brief		��ȡ��ģ�鳣��·��������ָ����ȡ����
*@param		nPathType		[IN]	·�����͡�0.��ǰ����Ŀ¼�� 1.Ӧ�ó���·����
*@param		pszPathBuf		[OUT]	·����ŵĻ���������󳤶�Ϊ PC_MAX_PATH
*@return	�����룬��PC_Lib.h
*/
int  PCGetSelfPath(int nPathType, char pszPathBuf[PC_MAX_PATH]);

/**
*@brief		�ж�ĳ����Դ����Ŀ¼���ļ��ȣ��Ƿ����
*@param		nResType		[IN]	��Դ���͡�0.Ŀ¼�� 1.�ļ���
*@param		pszPath			[IN]	��Դ·������󳤶�Ϊ PC_MAX_PATH
*@return	0�������óɹ�����Դ�����ڣ�1�������óɹ�����Դ���ڡ�ʧ�ܷ���<0�Ĵ����룬��PC_Lib.h
*/
int  PCResExist(int nResType, const char * pszPath);

/**
*@brief		�����ļ��У����Դ����༶�ļ��С����Ҫ�������ļ����Ѿ����ڣ�Ҳ���سɹ���
*@param		pszPath			[IN]	Ҫ�������ļ��е�·������󳤶�Ϊ PC_MAX_PATH
*@return	�����룬��PC_Lib.h
*/
int  PCMkdirRecursive(const char * pszPath);

/**
*@brief		ɾ���ļ��У�����ɾ���༶�ļ��кͷǿ��ļ��С�
*@param		pszPath			[IN]	Ҫɾ�����ļ��е�·������󳤶�Ϊ PC_MAX_PATH
*@return	�����룬��PC_Lib.h
*/
int  PCRemoveRecursive(const char * pszPath);

/**
*@brief		��ȡ�ļ���С����λ���ֽڣ�
*@param		pszPath			[IN]	�ļ�·������󳤶�Ϊ PC_MAX_PATH
*@return	�ɹ�ʱΪ>=0���ļ��ֽڴ�С��<0ʱΪ�����룬��PC_Lib.h
*/
long long PCGetFileSize(const char * pszPath);

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCSYSTEMUTIL_H_)
