#if !defined(_PCUTILCHECKSUM_H_)
#define _PCUTILCHECKSUM_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		基本数据完整性校验算法的封装。
*/

//校验算法宏
#define	PC_ALGO_MD5				(0)		/*结果 16 字节*/
#define	PC_ALGO_SHA1			(1)		/*结果 20 字节*/
#define	PC_ALGO_SHA224			(2)		/*结果 28 字节*/
#define	PC_ALGO_SHA256			(3)		/*结果 32 字节*/
#define	PC_ALGO_SHA384			(4)		/*结果 48 字节*/
#define	PC_ALGO_SHA512			(5)		/*结果 64 字节*/

/**
*@brief		计算字节数组的校验值，输入值和输出值均为原始值。
*@param		nAlgo		[IN]	算法，见文件头校验算法宏定义
*@param		pszSrc		[IN]	输入的数据
*@param		nSrcLen		[IN]	输入的数据长度
*@param		pszMd		[OUT]	输出的数据缓冲区，外部必须保证分配的空间足够
*@return	成功时返回输出数据的长度；失败时返回<0的错误码，见PC_Lib.h
*/
int  PCGetMd(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, unsigned char *pszMd);

/**
*@brief		计算文件的校验值，输出值为原始值。
*@param		nAlgo		[IN]	算法，见文件头校验算法宏定义
*@param		pszSrc		[IN]	输入的数据
*@param		nSrcLen		[IN]	输入的数据长度
*@param		pszMd		[OUT]	输出的数据缓冲区，外部必须保证分配的空间足够
*@return	成功时返回输出数据的长度；失败时返回<0的错误码，见PC_Lib.h
*/
int  PCGetFileMd(int nAlgo, const char *pszPath, unsigned char *pszMd);

/**
*@brief		计算字节数组的HMAC值，输入值和输出值均为原始值。
*@param		nAlgo		[IN]	算法，见文件头校验算法宏定义
*@param		pszSrc		[IN]	输入的数据
*@param		nSrcLen		[IN]	输入的数据长度
*@param		pszKey		[IN]	密钥
*@param		nKeyLen		[IN]	密钥长度
*@param		pszHMac		[OUT]	输出的数据缓冲区，必须大于或等于16字节
*@return	成功时返回输出数据的长度；失败时返回<0的错误码，见PC_Lib.h
*/
int  PCGetHMac(int nAlgo, const unsigned char *pszSrc, size_t nSrcLen, const unsigned char *pszKey, unsigned int nKeyLen, unsigned char *pszHMac);

/**
*@brief		计算字节数组的CRC32或Adler-32校验和。Adler-32比CRC更快，和CRC一样可靠。输入值为原始值。
*@param		pszSrc		[IN]	输入的数据
*@param		nSrcLen		[IN]	输入的数据长度
*@param		bCrc32Mode	[IN]	是否为CRC32模式。true CRC模式 ； false Adler-32模式
*@return	返回CRC32值或Adler-32值，4字节的长整数。如果输入数据为NULL或长度为空，返回0
*/
unsigned long  PCGetCOA32(const unsigned char *pszSrc, unsigned int nSrcLen, bool bCrc32Mode = true);

/**
*@brief		计算文件的CRC32或Adler-32校验和。
*@param		pszPath		[IN]	文件路径
*@param		ulCOA32		[OUT]	计算后的CRC32或Adler-32，输出为4字节的长整数
*@param		bCrc32Mode	[IN]	是否为CRC32模式。true CRC模式 ； false Adler-32模式
*@return	错误码，见PC_Lib.h
*/
int  PCGetFileCOA32(const char *pszPath, unsigned long &ulCOA32, bool bCrc32Mode = true);

/**
*@brief		计算字节数组的异或校验值。
*@param		pszSrc		[IN]	输入的数据
*@param		nSrcLen		[IN]	输入的数据长度
*@return	返回1字节的异或校验值。如果输入数据为NULL或长度为空，返回0
*/
unsigned char  PCGetXorCheckSum(const unsigned char *pszSrc, unsigned int nSrcLen);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILCHECKSUM_H_)
