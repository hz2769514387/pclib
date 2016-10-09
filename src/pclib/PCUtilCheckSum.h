#if !defined(_PCUTILCHECKSUM_H_)
#define _PCUTILCHECKSUM_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		��������������У���㷨�ķ�װ��
*/

//У���㷨��
#define	PC_ALGO_MD5				(0)		/*��� 16 �ֽ�*/
#define	PC_ALGO_SHA1			(1)		/*��� 20 �ֽ�*/
#define	PC_ALGO_SHA224			(2)		/*��� 28 �ֽ�*/
#define	PC_ALGO_SHA256			(3)		/*��� 32 �ֽ�*/
#define	PC_ALGO_SHA384			(4)		/*��� 48 �ֽ�*/
#define	PC_ALGO_SHA512			(5)		/*��� 64 �ֽ�*/

/**
*@brief		�����ֽ������У��ֵ������ֵ�����ֵ��Ϊԭʼֵ��
*@param		nAlgo		[IN]	�㷨�����ļ�ͷУ���㷨�궨��
*@param		pszSrc		[IN]	���������
*@param		nSrcLen		[IN]	��������ݳ���
*@param		pszMd		[OUT]	��������ݻ��������ⲿ���뱣֤����Ŀռ��㹻
*@return	�ɹ�ʱ����������ݵĳ��ȣ�ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCGetMd(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, unsigned char *pszMd);

/**
*@brief		�����ļ���У��ֵ�����ֵΪԭʼֵ��
*@param		nAlgo		[IN]	�㷨�����ļ�ͷУ���㷨�궨��
*@param		pszSrc		[IN]	���������
*@param		nSrcLen		[IN]	��������ݳ���
*@param		pszMd		[OUT]	��������ݻ��������ⲿ���뱣֤����Ŀռ��㹻
*@return	�ɹ�ʱ����������ݵĳ��ȣ�ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCGetFileMd(int nAlgo, const char *pszPath, unsigned char *pszMd);

/**
*@brief		�����ֽ������HMACֵ������ֵ�����ֵ��Ϊԭʼֵ��
*@param		nAlgo		[IN]	�㷨�����ļ�ͷУ���㷨�궨��
*@param		pszSrc		[IN]	���������
*@param		nSrcLen		[IN]	��������ݳ���
*@param		pszKey		[IN]	��Կ
*@param		nKeyLen		[IN]	��Կ����
*@param		pszHMac		[OUT]	��������ݻ�������������ڻ����16�ֽ�
*@return	�ɹ�ʱ����������ݵĳ��ȣ�ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCGetHMac(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszHMac);

/**
*@brief		�����ֽ������CRC32��Adler-32У��͡�Adler-32��CRC���죬��CRCһ���ɿ�������ֵΪԭʼֵ��
*@param		pszSrc		[IN]	���������
*@param		nSrcLen		[IN]	��������ݳ���
*@param		bCrc32Mode	[IN]	�Ƿ�ΪCRC32ģʽ��true CRCģʽ �� false Adler-32ģʽ
*@return	����CRC32ֵ��Adler-32ֵ��4�ֽڵĳ������������������ΪNULL�򳤶�Ϊ�գ�����0
*/
unsigned long  PCGetCOA32(const unsigned char *pszSrc, unsigned int nSrcLen, bool bCrc32Mode = true);

/**
*@brief		�����ļ���CRC32��Adler-32У��͡�
*@param		pszPath		[IN]	�ļ�·��
*@param		ulCOA32		[OUT]	������CRC32��Adler-32�����Ϊ4�ֽڵĳ�����
*@param		bCrc32Mode	[IN]	�Ƿ�ΪCRC32ģʽ��true CRCģʽ �� false Adler-32ģʽ
*@return	�����룬��PC_Lib.h
*/
int  PCGetFileCOA32(const char *pszPath, unsigned long &ulCOA32, bool bCrc32Mode = true);

/**
*@brief		�����ֽ���������У��ֵ��
*@param		pszSrc		[IN]	���������
*@param		nSrcLen		[IN]	��������ݳ���
*@return	����1�ֽڵ����У��ֵ�������������ΪNULL�򳤶�Ϊ�գ�����0
*/
unsigned char  PCGetXorCheckSum(const unsigned char *pszSrc, unsigned int nSrcLen);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILCHECKSUM_H_)
