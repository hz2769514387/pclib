#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCUtilString.h"
#include "PCMemory.h" 
#include "PCUtilSymEncrypt.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


/**
*@brief		内部函数，将nAlgo和nKeyLen转换为回调函数地址。
*@param		nAlgo	[IN]	算法及其工作模式。如 PC_3DES_WORKMODE_ECB 等。
*@param		nKeyLen	[IN]	密钥长度。
*@return	成功时返回回调地址；失败时返回NULL
*/
const EVP_CIPHER *  PCGetFunAddrByAlgo(int nAlgo, unsigned int nKeyLen)
{
	switch (nAlgo)
	{
		//3DES部分
	case PC_3DES_ECB:
		if (nKeyLen == 8)
		{
			return EVP_des_ecb();
		}
		else if (nKeyLen == 16)
		{
			return EVP_des_ede_ecb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_ecb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_3DES_CBC:
		if (nKeyLen == 8)
		{
			return EVP_des_cbc();
		}
		else if (nKeyLen == 16)
		{
			return EVP_des_ede_cbc();
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_cbc();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_3DES_CFB:
		if (nKeyLen == 8)
		{
			return EVP_des_cfb();
		}
		else if (nKeyLen == 16)
		{
			return EVP_des_ede_cfb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_cfb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_3DES_OFB:
		if (nKeyLen == 8)
		{
			return EVP_des_ofb();
		}
		else if (nKeyLen == 16)
		{
			return EVP_des_ede_ofb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_ofb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_3DES_CFB1:
		if (nKeyLen == 8)
		{
			return EVP_des_cfb1();
		}
		else if (nKeyLen == 16)
		{
			//不支持
			PC_ERROR_LOG("nKeyLen(%d) CFB1 NOT SUPPORTED . ", nKeyLen);
			return NULL;
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_cfb1();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_3DES_CFB8:
		if (nKeyLen == 8)
		{
			return EVP_des_cfb8();
		}
		else if (nKeyLen == 16)
		{
			//不支持
			PC_ERROR_LOG("nKeyLen(%d) CFB8 NOT SUPPORTED . ", nKeyLen);
			return NULL;
		}
		else if (nKeyLen == 24)
		{
			return EVP_des_ede3_cfb8();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}

		//AES部分
	case PC_AES_ECB:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_ecb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_ecb();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_ecb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_AES_CBC:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_cbc();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_cbc();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_cbc();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_AES_CFB:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_cfb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_cfb();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_cfb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_AES_OFB:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_ofb();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_ofb();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_ofb();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_AES_CFB1:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_cfb1();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_cfb1();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_cfb1();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	case PC_AES_CFB8:
		if (nKeyLen == 16)
		{
			return EVP_aes_128_cfb8();
		}
		else if (nKeyLen == 24)
		{
			return EVP_aes_192_cfb8();
		}
		else if (nKeyLen == 32)
		{
			return EVP_aes_256_cfb8();
		}
		else
		{
			PC_ERROR_LOG("nKeyLen(%d) nAlgo(%d) NOT SUPPORTED . ", nKeyLen, nAlgo);
			return NULL;
		}
	default:
		PC_ERROR_LOG("nAlgo(%d) nKeyLen(%d) INVALID . ", nAlgo, nKeyLen);
		return NULL;
	}
}
     

int  PCSymEncypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszKey == NULL || pszDest == NULL || nDestBufLen < (nSrcLen + PC_MAX_BLOCK_SIZE))
	{
		PC_ERROR_LOG("params error! some value is NULL");
		return PC_RESULT_PARAM;
	}
	//AES和3DES的最后分组长度不同
	int nBlockSize = 0;
	if (nAlgo >= PC_3DES_ECB && nAlgo <= PC_3DES_CFB8)
	{
		nBlockSize = PC_3DES_BLOCK_SIZE;
	}
	else if (nAlgo >= PC_AES_ECB && nAlgo <= PC_AES_CFB8)
	{
		nBlockSize = PC_AES_BLOCK_SIZE;
	}
	else
	{
		PC_ERROR_LOG("params error!  nAlgo is %d", nAlgo);
		return PC_RESULT_PARAM;
	}
	
	//工作模式
	const EVP_CIPHER * pAlgoImpl = PCGetFunAddrByAlgo(nAlgo, nKeyLen);
	if (NULL == pAlgoImpl)
	{
		return PC_RESULT_PARAM;
	}

	//输入处理
	unsigned char pszLastBlock[PC_MAX_BLOCK_SIZE] = { 0 };
	int nBlockedSrcLen = PCGetPaddingLastBlock(pszPaddingMode, pszSrc, nSrcLen, nBlockSize, pszLastBlock);
	if (nBlockedSrcLen < 0)
	{
		return nBlockedSrcLen;
	}

	//IV处理
	unsigned char szRealIV[PC_MAX_BLOCK_SIZE] = { 0 };
	memset(szRealIV, 0, PC_MAX_BLOCK_SIZE);
	if (pszIV)
	{
		memcpy(szRealIV, pszIV, nBlockSize);
	}

	EVP_CIPHER_CTX m_context;
	EVP_CIPHER_CTX_init(&m_context);
	std::shared_ptr<EVP_CIPHER_CTX> pEvpPtr(&m_context, EVP_CIPHER_CTX_cleanup);
	
	if (!EVP_EncryptInit_ex(pEvpPtr.get(), pAlgoImpl, NULL, pszKey, szRealIV))
	{
		PC_ERROR_LOG("EVP_EncryptInit_ex error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	if (!EVP_CIPHER_CTX_set_key_length(pEvpPtr.get(), nKeyLen))
	{
		PC_ERROR_LOG("EVP_CIPHER_CTX_set_key_length error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	if (!EVP_CIPHER_CTX_set_padding(pEvpPtr.get(), 0))
	{
		PC_ERROR_LOG("EVP_CIPHER_CTX_set_padding error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	
	int nDestLen = 0;
	if (nBlockedSrcLen > 0)
	{
		if (!EVP_EncryptUpdate(pEvpPtr.get(), pszDest, &nDestLen, pszSrc, nBlockedSrcLen))
		{
			PC_ERROR_LOG("EVP_EncryptUpdate error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
			return PC_RESULT_SYSERROR;
		}
	}
	int nLastBlockLen;
	if (!EVP_EncryptUpdate(pEvpPtr.get(), pszDest + nDestLen, &nLastBlockLen, pszLastBlock, nBlockSize))
	{
		PC_ERROR_LOG("EVP_EncryptUpdate error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	int nFinalLen;
	if (!EVP_EncryptFinal_ex(pEvpPtr.get(), (pszDest + nDestLen + nLastBlockLen), &nFinalLen))
	{
		PC_ERROR_LOG("EVP_EncryptFinal_ex error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}

	nDestLen = nDestLen + nLastBlockLen + nFinalLen;
	return nDestLen;
}

int  PCSymDecypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszKey == NULL || pszDest == NULL || nDestBufLen < (nSrcLen + PC_MAX_BLOCK_SIZE))
	{
		PC_ERROR_LOG("params error! some value is NULL");
		return PC_RESULT_PARAM;
	}

	//AES和3DES的最后分组长度不同
	int nBlockSize = 0;
	if (nAlgo >= PC_3DES_ECB && nAlgo <= PC_3DES_CFB8)
	{
		nBlockSize = PC_3DES_BLOCK_SIZE;
	}
	else if (nAlgo >= PC_AES_ECB && nAlgo <= PC_AES_CFB8)
	{
		nBlockSize = PC_AES_BLOCK_SIZE;
	}
	else
	{
		PC_ERROR_LOG("params error!  nAlgo is %d", nAlgo);
		return PC_RESULT_PARAM;
	}

	//解密时输入数据长度如果不为BLOCK_SIZE的倍数，说明这个数据不是从加密得来的。非法。
	if (nSrcLen % nBlockSize != 0)
	{
		PC_ERROR_LOG("INPUT DATA LEN(%u) is INVALID, nAlgo = %d", nSrcLen, nAlgo);
		return PC_RESULT_PARAM;
	}

	//工作模式
	const EVP_CIPHER * pAlgoImpl = PCGetFunAddrByAlgo(nAlgo, nKeyLen);
	if (NULL == pAlgoImpl)
	{
		return PC_RESULT_PARAM;
	}

	//IV处理
	unsigned char szRealIV[PC_MAX_BLOCK_SIZE] = { 0 };
	memset(szRealIV, 0, PC_MAX_BLOCK_SIZE);
	if (pszIV)
	{
		memcpy(szRealIV, pszIV, nBlockSize);
	}

	EVP_CIPHER_CTX m_context;
	EVP_CIPHER_CTX_init(&m_context);
	std::shared_ptr<EVP_CIPHER_CTX> pEvpPtr(&m_context, EVP_CIPHER_CTX_cleanup);

	if (!EVP_DecryptInit_ex(pEvpPtr.get(), pAlgoImpl, NULL, pszKey, szRealIV))
	{
		PC_ERROR_LOG("EVP_DecryptInit_ex error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	if (!EVP_CIPHER_CTX_set_key_length(pEvpPtr.get(), nKeyLen))
	{
		PC_ERROR_LOG("EVP_CIPHER_CTX_set_key_length error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	if (!EVP_CIPHER_CTX_set_padding(pEvpPtr.get(), 0))
	{
		PC_ERROR_LOG("EVP_CIPHER_CTX_set_padding error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	int nLastBlockLen;
	if (!EVP_DecryptUpdate(pEvpPtr.get(), pszDest, &nLastBlockLen, pszSrc, nSrcLen))
	{
		PC_ERROR_LOG("EVP_DecryptUpdate error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}
	int nFinalLen;
	if (!EVP_DecryptFinal_ex(pEvpPtr.get(), pszDest + nLastBlockLen, &nFinalLen))
	{
		PC_ERROR_LOG("EVP_EncryptFinal_ex error!nSrcLen=%u, nKeyLen=%u", nSrcLen, nKeyLen);
		return PC_RESULT_SYSERROR;
	}

	unsigned int nDestLen = nLastBlockLen + nFinalLen;
	return PCRemovePadding(pszPaddingMode, pszDest, nDestLen, nBlockSize);
}




int  PCGetPaddingLastBlock(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen, unsigned char *pszDest)
{
	if (pszSrc == NULL || nSrcLen == 0 || nBlockLen == 0 || nBlockLen  > 0xFF || pszDest == NULL)
	{
		PC_ERROR_LOG("params error! some value is NULL");
		return PC_RESULT_PARAM;
	}

	//从数据头开始的原始数据分组后的长度
	unsigned int nBlockedSrcLen = 0;			
	if (0 == PCStrCaseCmp("PKCS7", pszPaddingMode))
	{
		unsigned int nLeft = nSrcLen % nBlockLen;	
		nBlockedSrcLen = nSrcLen - nLeft;
		memset(pszDest, (unsigned char)(nBlockLen - nLeft), nBlockLen);
		if (nLeft != 0)
		{
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nLeft);
		}
	}
	else if (0 == PCStrCaseCmp("ANSIX923", pszPaddingMode))
	{
		unsigned int nLeft = nSrcLen % nBlockLen;	
		nBlockedSrcLen = nSrcLen - nLeft;
		memset(pszDest, 0, nBlockLen);
		if (nLeft != 0)
		{
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nLeft);
		}
		pszDest[nBlockLen - 1] = (unsigned char)(nBlockLen - nLeft);
	}
	else if (0 == PCStrCaseCmp("ISO10126", pszPaddingMode))
	{
		unsigned int nLeft = nSrcLen % nBlockLen;	
		nBlockedSrcLen = nSrcLen - nLeft;
		PCGetRandomBytes(pszDest, nBlockLen);
		if (nLeft != 0)
		{
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nLeft);
		}
		pszDest[nBlockLen - 1] = (unsigned char)(nBlockLen - nLeft);
	}
	else if (0 == PCStrCaseCmp("PBOC", pszPaddingMode))
	{
		unsigned int nLeftPBOC = (nSrcLen + 1) % nBlockLen;	
		if (nLeftPBOC != 0)
		{
			nBlockedSrcLen = nSrcLen - (nLeftPBOC - 1);
			memset(pszDest, 0, nBlockLen);
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nLeftPBOC - 1);
			pszDest[nLeftPBOC - 1] = 0x80;
		}
		else
		{
			nBlockedSrcLen = nSrcLen - (nBlockLen - 1);
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nBlockLen - 1);
			pszDest[nBlockLen - 1] = 0x80;
		}
	}
	else
	{
		//指定字节填充
		if (strlen(pszPaddingMode) != 2 )
		{
			PC_ERROR_LOG("params error! pszPaddingMode = %s", pszPaddingMode);
			return PC_RESULT_PARAM;
		}
		unsigned char pszDestPaddingByte[2] ;
		if (0 > PCDispHexStr2Bytes(pszPaddingMode, pszDestPaddingByte, 2))
		{
			return PC_RESULT_PARAM;
		}

		unsigned int nLeft = nSrcLen % nBlockLen;
		if (nLeft != 0)
		{
			nBlockedSrcLen = nSrcLen - nLeft;
			memset(pszDest, pszDestPaddingByte[0], nBlockLen);
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nLeft);
		}
		else
		{
			nBlockedSrcLen = nSrcLen - nBlockLen;
			memcpy(pszDest, pszSrc + nBlockedSrcLen, nBlockLen);
		}
	}
	return nBlockedSrcLen;
}

int  PCRemovePadding(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen)
{
	if (pszSrc == NULL || nSrcLen == 0 || nBlockLen == 0 || nBlockLen  > 0xFF)
	{
		PC_ERROR_LOG("params error! some value is NULL");
		return PC_RESULT_PARAM;
	}

	//解除填充后的数据长度
	unsigned int nSrcRealDataLen = 0;
	if(  (0 == PCStrCaseCmp("PKCS7", pszPaddingMode))		|| 
		 (0 == PCStrCaseCmp("ANSIX923", pszPaddingMode))	|| 
		 (0 == PCStrCaseCmp("ISO10126", pszPaddingMode))	)
	{
		unsigned int nPaddingLen = pszSrc[nSrcLen-1];
		if (nPaddingLen > nBlockLen || nPaddingLen > nSrcLen)
		{
			PC_ERROR_LOG("format error! nPaddingLen (%u) > nBlockLen(%u) or nSrcLen(%u)", nPaddingLen, nBlockLen,nSrcLen);
			return PC_RESULT_FORMATERROR;
		}
		nSrcRealDataLen = nSrcLen - nPaddingLen;
	}
	else if (0 == PCStrCaseCmp("PBOC", pszPaddingMode))
	{
		//从后往前查找0x80
		int nPos80 = -1;
		for (int i = nSrcLen - 1; i >= 0; i--)
		{
			if (pszSrc[i] == 0x80)
			{
				nPos80 = i;
				break;
			}
		}
		if (-1 == nPos80)
		{
			PC_ERROR_LOG("format error! NOT FIND 0x80");
			return PC_RESULT_FORMATERROR;
		}
		nSrcRealDataLen = nPos80;
	}
	else
	{
		//指定字节填充
		if (strlen(pszPaddingMode) != 2)
		{
			PC_ERROR_LOG("params error! pszPaddingMode = %s", pszPaddingMode);
			return PC_RESULT_PARAM;
		}
		unsigned char pszDestPaddingByte[2];
		if (0 > PCDispHexStr2Bytes(pszPaddingMode, pszDestPaddingByte, 2))
		{
			return PC_RESULT_PARAM;
		}
		//指定字节填充无法判断，不移除
		nSrcRealDataLen = nSrcLen;
	}
	return nSrcRealDataLen;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////