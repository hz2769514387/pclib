#if !defined(_PCUTILSYMENCRYPT_H_)
#define _PCUTILSYMENCRYPT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		�ԳƼ����㷨�ķ�װ��
*/

//�ֿ����Ŀ��С
#define PC_MAX_BLOCK_SIZE		(16)
#define PC_3DES_BLOCK_SIZE		(8)
#define PC_AES_BLOCK_SIZE		(16)

//�ԳƼ����㷨���乤��ģʽ
#define PC_3DES_ECB				(0)
#define PC_3DES_CBC				(1)
#define PC_3DES_CFB				(2)		//CFB64
#define PC_3DES_OFB				(3)
#define PC_3DES_CFB1			(4)
#define PC_3DES_CFB8			(5)
#define PC_AES_ECB				(6)
#define PC_AES_CBC				(7)
#define PC_AES_CFB				(8)		//CFB128
#define PC_AES_OFB				(9)
#define PC_AES_CFB1				(10)
#define PC_AES_CFB8				(11)

/**
*@brief		�ԳƼ��ܽ����㷨����3desʱ����e-d-eģʽ���㡣����ֵ�����ֵ��Ϊԭʼֵ��
*@param		nAlgo			[IN]	�㷨���乤��ģʽ���� PC_3DES_ECB �ȡ�
*@param		pszSrc			[IN]	���������
*@param		nSrcLen			[IN]	��������ݳ���
*@param		pszKey			[IN]	��Կ
*@param		nKeyLen			[IN]	��Կ����,3DES����Կ����ֻ��Ϊ8��16��24�ֽڣ�AES��Կ����Ϊ16��24��32�ֽ�
*@param		pszDest			[OUT]	������ݻ�����
*@param		nDestBufLen		[IN]	������ݻ��������ȣ����������ȹ涨������ڣ�nSrcLen + PC_MAX_BLOCK_SIZE��
*@param		pszPaddingMode	[IN]	���ģʽ����pad_mode=""��NULLʱ��ʹ��ָ���ֽ�0x00��䡣���ģʽ��PCGetPaddingLastBlock������
*@param		pszIV			[IN]	��ʼ����(���ȱ���Ϊ8(3DES)��16(AES)�ֽ�)����pszIV=NULLʱ��Ĭ��ʹ��ȫ0x00��Ϊ��ʼ������
*@return	�ɹ�ʱ����������ݵĳ��ȣ�ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCSymEncypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV);
int  PCSymDecypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV);


/**
*@brief		�������ģʽ��ȡ���һ����������ݺ�ԭʼ���ݷ����Ĵ�С��
*@param		pszPaddingMode	[IN]	���ģʽ����pad_mode=""��NULLʱ��ʹ��ָ���ֽ�0x00��䡣���ģʽ���£�
*@									"00"-"FF"	ָ���ֽ���䣺����ַ�����һ���ֽ�������ɣ�ÿ���ֽڿ���ʹ��pszPaddingMode��Ӧֵ0x00~0xff��
*@									"PKCS7"		PKCS7ģʽ��䣺����ַ�����һ���ֽ�������ɣ�ÿ���ֽ������ֽ����еĳ��ȡ�
*@									"ANSIX923"	ANSIX923ģʽ������ַ�����һ���ֽ�������ɣ����ֽ����е����һ���ֽ�����ֽ����еĳ��ȣ������ֽھ���������㡣
*@									"ISO10126"	ISO10126ģʽ������ַ�����һ���ֽ�������ɣ����ֽ����е����һ���ֽ�����ֽ����еĳ��ȣ������ֽ����������ݡ�
*@									"PBOC"		PBOCģʽ�����������ַ���ǿ�����0x80�������ʱ���Ȼ�δ�ﵽnBlockLen����С�����������0x00��nBlockLen����С��������
*@param		pszSrc			[IN]	���������
*@param		nSrcLen			[IN]	��������ݳ���
*@param		nBlockLen		[IN]	����ĳ��ȣ���DES/3DESΪ8�ֽڣ�AESΪ16�ֽ�
*@param		pszDest			[OUT]	���һ����������ݻ����������������ȱ����/����nBlockLen�����غ�����ݳ���ΪnBlockLen�ֽڡ�
*@return	�ɹ�ʱ����ԭʼ���ݷ����Ĵ�С��ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCGetPaddingLastBlock(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen, unsigned char *pszDest);

/**
*@brief		�������ģʽ��ȡ���һ����������ݺ�ԭʼ���ݷ����Ĵ�С��
*@param		pszPaddingMode	[IN]	���ģʽ����pad_mode=""��NULLʱ����Ϊ��ָ���ֽ�0x00��䡣���ģʽ��PCGetPaddingLastBlock������
*@param		pszSrc			[IN]	Ҫ�����������
*@param		nSrcLen			[IN]	Ҫ����������ݳ���
*@param		nBlockLen		[IN]	����ĳ��ȣ���DES/3DESΪ8�ֽڣ�AESΪ16�ֽ�
*@return	�ɹ�ʱ�����������ݳ��ȡ�ָ���ֽ���䲻�ܱ�ȥ��������Ȼ��ɹ���ʧ��ʱ����<0�Ĵ����룬��PC_Lib.h
*/
int  PCRemovePadding(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILSYMENCRYPT_H_)
