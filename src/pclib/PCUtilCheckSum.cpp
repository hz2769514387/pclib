#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCUtilString.h"
#include "PCMemory.h" 
#include "PCUtilCheckSum.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		У���ڲ���������nAlgoת��Ϊ��ʵ�㷨�ص�������ַ��
*@param		nWorkMode	[IN]	����ģʽ���� PC_3DES_WORKMODE_ECB �ȡ�
*@param		nKeyLen		[IN]	��Կ���ȡ�
*@return	�ɹ�ʱ���ػص���ַ��ʧ��ʱ����NULL
*/
const EVP_MD *  PCGetCheckSumFuncAddr(int nAlgo)
{
	switch (nAlgo)
	{
	case PC_ALGO_MD5:
		return EVP_md5();
	case PC_ALGO_SHA1:
		return EVP_sha1();
	case PC_ALGO_SHA224:
		return EVP_sha224();
	case PC_ALGO_SHA256:
		return EVP_sha256();
	case PC_ALGO_SHA384:
		return EVP_sha384();
	case PC_ALGO_SHA512:
		return EVP_sha512();
	default:
		PC_ERROR_LOG("param error!nAlgo = %d", nAlgo);
		return NULL;
	}
}

int  PCGetMd(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, unsigned char *pszMd)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszMd == NULL)
	{
		PC_ERROR_LOG("param error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_PARAM;
	}

	//�����㷨
	const EVP_MD *mdAlgo = PCGetCheckSumFuncAddr(nAlgo);
	if (NULL == mdAlgo)
	{
		return PC_RESULT_PARAM;
	}

	EVP_MD_CTX m_context;
	EVP_MD_CTX_init(&m_context);
	std::shared_ptr<EVP_MD_CTX> pMdPtr(&m_context, EVP_MD_CTX_cleanup);
	if (!EVP_DigestInit_ex(pMdPtr.get(), mdAlgo, NULL))
	{
		PC_ERROR_LOG("EVP_DigestInit_ex error!nSrcLen=%u, algo=%d", nSrcLen, nAlgo);
		return PC_RESULT_SYSERROR;
	}
	if (!EVP_DigestUpdate(pMdPtr.get(), pszSrc, nSrcLen))
	{
		PC_ERROR_LOG("EVP_DigestUpdate error!nSrcLen=%u, algo=%d", nSrcLen, nAlgo);
		return PC_RESULT_SYSERROR;
	}
	unsigned int nMdLen;
	if (!EVP_DigestFinal_ex(pMdPtr.get(), pszMd, &nMdLen))
	{
		PC_ERROR_LOG("EVP_DigestFinal_ex error!nSrcLen=%u, algo=%d", nSrcLen, nAlgo);
		return PC_RESULT_SYSERROR;
	}

	return nMdLen;
}

int  PCGetFileMd(int nAlgo, const char *pszPath, unsigned char *pszMd)
{
	if (pszPath == NULL || pszMd == NULL)
	{
		PC_ERROR_LOG("param error!pszPath=%s", pszPath);
		return PC_RESULT_PARAM;
	}

	//�����㷨
	const EVP_MD *mdAlgo = PCGetCheckSumFuncAddr(nAlgo);
	if (NULL == mdAlgo)
	{
		return PC_RESULT_PARAM;
	}

	EVP_MD_CTX m_context;
	EVP_MD_CTX_init(&m_context);
	std::shared_ptr<EVP_MD_CTX> pMdPtr(&m_context, EVP_MD_CTX_cleanup);
	if (!EVP_DigestInit_ex(pMdPtr.get(), mdAlgo, NULL))
	{
		PC_ERROR_LOG("EVP_DigestInit_ex error!pszPath=%s, algo=%d", pszPath, nAlgo);
		return PC_RESULT_SYSERROR;
	}

	//���ļ�
	FILE* fp = fopen(pszPath, "rb");
	if (NULL == fp)
	{
		PC_ERROR_LOG("fopen(%s) = NULL!", pszPath);
		return PC_RESULT_FILEOPEN;
	}
	std::shared_ptr<FILE> fAgent(fp, fclose);

	//ѭ�����ļ�������
	size_t			nReadLen = 0;
    unsigned char	szArrayBuf[4096] = { 0 };

	while (true)
	{
		nReadLen = fread(szArrayBuf, 1, 4096, fAgent.get());
		if (nReadLen == 0)
		{
			//���ļ�β
			break;
		}

		if (!EVP_DigestUpdate(pMdPtr.get(), szArrayBuf, nReadLen))
		{
			PC_ERROR_LOG("EVP_DigestUpdate error!pszPath=%s, algo=%d", pszPath, nAlgo);
			return PC_RESULT_SYSERROR;
		}
	} 

	unsigned int nMdLen;
	if (!EVP_DigestFinal_ex(pMdPtr.get(), pszMd, &nMdLen))
	{
		PC_ERROR_LOG("EVP_DigestFinal_ex error!pszPath=%s, algo=%d", pszPath, nAlgo);
		return PC_RESULT_SYSERROR;
	}
	return nMdLen;
}

int  PCGetHMac(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszHMac)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszKey == NULL || nKeyLen == 0 || pszHMac == NULL)
	{
		PC_ERROR_LOG("param error!nAlgo = %d, nSrcLen=%u, nKeyLen=%u", nAlgo, nSrcLen, nKeyLen);
		return PC_RESULT_PARAM;
	}	
	//�����㷨
	const EVP_MD *mdAlgo = PCGetCheckSumFuncAddr(nAlgo);
	if (NULL == mdAlgo)
	{
		return PC_RESULT_PARAM;
	}

	HMAC_CTX m_context;
	HMAC_CTX_init(&m_context);
	std::shared_ptr<HMAC_CTX> pHMacPtr(&m_context, HMAC_CTX_cleanup);

	if (!HMAC_Init_ex(pHMacPtr.get(), pszKey, nKeyLen, mdAlgo, NULL))
	{
		PC_ERROR_LOG("HMAC_Init_ex error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	if (!HMAC_Update(pHMacPtr.get(), pszSrc, nSrcLen))
	{
		PC_ERROR_LOG("HMAC_Update error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	unsigned int nHmacLen;
	if (!HMAC_Final(pHMacPtr.get(), pszHMac, &nHmacLen))
	{
		PC_ERROR_LOG("HMAC_Final error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}

	return nHmacLen;

	/*
	//MD5 HMAC�ֶ��������ʾ��
	//HMAC MD5��������ݿ�B�ĳ���
	#define PC_HMAC_MD5_BLOCK_LEN		(64)

	//ʵ��ʹ�õ����ݶ��� 
	unsigned char szRealKey[PC_HMAC_MD5_BLOCK_LEN];
	unsigned char szTmpHMac[PC_HMAC_MD5_BLOCK_LEN];
	unsigned char szIpad[PC_HMAC_MD5_BLOCK_LEN];
	unsigned char szOpad[PC_HMAC_MD5_BLOCK_LEN];
	memset(szIpad, 0x36, PC_HMAC_MD5_BLOCK_LEN);
	memset(szOpad, 0x5C, PC_HMAC_MD5_BLOCK_LEN);
	memset(szRealKey, 0x00, PC_HMAC_MD5_BLOCK_LEN);
	memset(szTmpHMac, 0x00, PC_HMAC_MD5_BLOCK_LEN);
	
	//ʵ����Կ���� 
	if (nKeyLen > 64)
	{
		int nRet = PCGetMd5(pszKey, nKeyLen, szTmpHMac);
		if (nRet < 0)
		{
			return nRet;
		}
		memcpy(szRealKey, szTmpHMac, 16);
	}
	else
	{
		memcpy(szRealKey, pszKey, nKeyLen);
	}

	//���봦�� 
	int nRet = PCBytesXor(szRealKey, szIpad, PC_HMAC_MD5_BLOCK_LEN);
	if (nRet != PC_RESULT_SUCCESS)
	{
		return nRet;
	}

	MD5_CTX context;
	nRet = MD5_Init(&context);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Init error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Update(&context, szIpad, PC_HMAC_MD5_BLOCK_LEN);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Update szIpad error!PC_HMAC_MD5_BLOCK_LEN=%u", PC_HMAC_MD5_BLOCK_LEN);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Update(&context, pszSrc, nSrcLen);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Update pszSrc error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Final(szTmpHMac, &context);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Final szTmpHMac error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}

	//��Կ���� 
	nRet = PCBytesXor(szRealKey, szOpad, PC_HMAC_MD5_BLOCK_LEN);
	if (nRet != PC_RESULT_SUCCESS)
	{
		return nRet;
	}

	//������봦��Ĳ��ֺ���Կ����Ĳ���������������mac����
	nRet = MD5_Init(&context);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Init error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Update(&context, szOpad, PC_HMAC_MD5_BLOCK_LEN);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Update szOpad error!PC_HMAC_MD5_BLOCK_LEN=%u", PC_HMAC_MD5_BLOCK_LEN);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Update(&context, szTmpHMac, PC_MD5_LEN);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Update szTmpHMac error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}
	nRet = MD5_Final(pszMd5, &context);
	if (0 == nRet)
	{
		PC_TRACE_LOG("MD5_Final pszMd5 error!nSrcLen=%u", nSrcLen);
		return PC_RESULT_SYSERROR;
	}
	return PC_RESULT_SUCCESS;
	*/
}


unsigned long  PCGetCOA32(const unsigned char *pszSrc, unsigned int nSrcLen, bool bCrc32Mode)
{
	if (pszSrc == NULL || nSrcLen == 0 )
	{
		PC_ERROR_LOG("param error!nSrcLen=%u", nSrcLen);
		return 0;
	}
	if (bCrc32Mode)
	{
		unsigned long ulCrc32 = crc32(0L, Z_NULL, 0);
		return crc32(ulCrc32, pszSrc, nSrcLen);
	}
	else
	{
		unsigned long ulAlder32 = adler32(0L, Z_NULL, 0);
		return adler32(ulAlder32, pszSrc, nSrcLen);
	}
}

int  PCGetFileCOA32(const char *pszPath, unsigned long &ulCOA32, bool bCrc32Mode)
{
	if (pszPath == NULL)
	{
		PC_ERROR_LOG("param error!");
		return PC_RESULT_PARAM;
	}

	if (bCrc32Mode)
	{
		ulCOA32 = crc32(0L, Z_NULL, 0);
	}
	else
	{
		ulCOA32 = adler32(0L, Z_NULL, 0);
	}

	//���ļ�
	FILE* fp = fopen(pszPath, "rb");
	if (NULL == fp)
	{
		PC_ERROR_LOG("fopen(%s) = NULL!", pszPath);
		return PC_RESULT_FILEOPEN;
	}
	std::shared_ptr<FILE> fAgent(fp, fclose);

	//ѭ�����ļ�������
	unsigned int	nReadLen = 0;
	unsigned char	szArrayBuf[4096] = { 0 };

	while (true)
	{
		nReadLen = (unsigned int)fread(szArrayBuf, 1, 4096, fAgent.get());
		if (nReadLen == 0)
		{
			//���ļ�β
			break;
		}

		if (bCrc32Mode)
		{
			ulCOA32 = crc32(ulCOA32, szArrayBuf, nReadLen);
		}
		else
		{
			ulCOA32 = adler32(ulCOA32, szArrayBuf, nReadLen);
		}
	}
	return PC_RESULT_SUCCESS;
}


unsigned char  PCGetXorCheckSum(const unsigned char *pszSrc, unsigned int nSrcLen)
{
	if (pszSrc == NULL || nSrcLen == 0)
	{
		PC_ERROR_LOG("param error!nSrcLen=%u", nSrcLen);
		return 0x00;
	}

	unsigned char ucXorCheckSum = 0x00;
	for (unsigned int i = 0; i < nSrcLen; i++)
	{
		ucXorCheckSum = ucXorCheckSum ^ pszSrc[i];
	}
	return ucXorCheckSum;
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////