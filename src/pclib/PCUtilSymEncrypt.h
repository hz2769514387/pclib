#if !defined(_PCUTILSYMENCRYPT_H_)
#define _PCUTILSYMENCRYPT_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		对称加密算法的封装。
*/

//分块计算的块大小
#define PC_MAX_BLOCK_SIZE		(16)
#define PC_3DES_BLOCK_SIZE		(8)
#define PC_AES_BLOCK_SIZE		(16)

//对称加密算法及其工作模式
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
*@brief		对称加密解密算法，当3des时采用e-d-e模式计算。输入值和输出值均为原始值。
*@param		nAlgo			[IN]	算法及其工作模式。如 PC_3DES_ECB 等。
*@param		pszSrc			[IN]	输入的数据
*@param		nSrcLen			[IN]	输入的数据长度
*@param		pszKey			[IN]	密钥
*@param		nKeyLen			[IN]	密钥长度,3DES的密钥长度只能为8、16、24字节，AES密钥长度为16、24、32字节
*@param		pszDest			[OUT]	输出数据缓冲区
*@param		nDestBufLen		[IN]	输出数据缓冲区长度，缓冲区长度规定必须大于（nSrcLen + PC_MAX_BLOCK_SIZE）
*@param		pszPaddingMode	[IN]	填充模式。当pad_mode=""或NULL时，使用指定字节0x00填充。填充模式见PCGetPaddingLastBlock函数。
*@param		pszIV			[IN]	初始向量(长度必须为8(3DES)或16(AES)字节)，当pszIV=NULL时，默认使用全0x00作为初始向量。
*@return	成功时返回输出数据的长度；失败时返回<0的错误码，见PC_Lib.h
*/
int  PCSymEncypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV);
int  PCSymDecypt(int nAlgo, const unsigned char *pszSrc, unsigned int nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszDest, unsigned int nDestBufLen, const char * pszPaddingMode, const unsigned char * pszIV);


/**
*@brief		根据填充模式获取最后一个分组的数据和原始数据分组后的大小。
*@param		pszPaddingMode	[IN]	填充模式。当pad_mode=""或NULL时，使用指定字节0x00填充。填充模式如下：
*@									"00"-"FF"	指定字节填充：填充字符串由一个字节序列组成，每个字节可以使用pszPaddingMode对应值0x00~0xff。
*@									"PKCS7"		PKCS7模式填充：填充字符串由一个字节序列组成，每个字节填充该字节序列的长度。
*@									"ANSIX923"	ANSIX923模式：填充字符串由一个字节序列组成，此字节序列的最后一个字节填充字节序列的长度，其余字节均填充数字零。
*@									"ISO10126"	ISO10126模式：填充字符串由一个字节序列组成，此字节序列的最后一个字节填充字节序列的长度，其余字节填充随机数据。
*@									"PBOC"		PBOC模式：将待加密字符串强制填充0x80，如果此时长度还未达到nBlockLen的最小整数倍则填充0x00到nBlockLen的最小整数倍。
*@param		pszSrc			[IN]	输入的数据
*@param		nSrcLen			[IN]	输入的数据长度
*@param		nBlockLen		[IN]	分组的长度，如DES/3DES为8字节，AES为16字节
*@param		pszDest			[OUT]	最后一个分组的数据缓冲区，缓冲区长度必须大/等于nBlockLen，返回后的数据长度为nBlockLen字节。
*@return	成功时返回原始数据分组后的大小。失败时返回<0的错误码，见PC_Lib.h
*/
int  PCGetPaddingLastBlock(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen, unsigned char *pszDest);

/**
*@brief		根据填充模式获取最后一个分组的数据和原始数据分组后的大小。
*@param		pszPaddingMode	[IN]	填充模式。当pad_mode=""或NULL时，认为是指定字节0x00填充。填充模式见PCGetPaddingLastBlock函数。
*@param		pszSrc			[IN]	要解除填充的数据
*@param		nSrcLen			[IN]	要解除填充的数据长度
*@param		nBlockLen		[IN]	分组的长度，如DES/3DES为8字节，AES为16字节
*@return	成功时解除填充后的数据长度。指定字节填充不能被去除但是仍然会成功。失败时返回<0的错误码，见PC_Lib.h
*/
int  PCRemovePadding(const char * pszPaddingMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILSYMENCRYPT_H_)
