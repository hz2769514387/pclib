#if !defined(_PCSTRINGUTIL_H)
#define _PCSTRINGUTIL_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


/**
*@brief		将字符串转换为小写
*@param		pszBuf		[IN]	待转换的字符串，调用该函数后pszBuf可能被改变
*@return	指向转换后的字符串的指针
*/
char* PCStrToLower(char *pszBuf);

/**
*@brief		将字符串转换为大写
*@param		pszBuf		[IN]	待转换的字符串，调用该函数后pszBuf可能被改变
*@return	指向转换后的字符串的指针
*/
char* PCStrToUpper(char *pszBuf);

/**
*@brief		不区分大小写的strstr函数
*@param		str1	[IN]	被查找目标，不能为NULL或空串
*@param		str2	[IN]	要查找对象，不能为NULL或空串
*@return	若str2是str1的子串，则返回str2在str1的首次出现的地址；如果str2不是str1的子串，则返回NULL。
*/
const char* PCStrStricase(const char* str1, const char* str2);

/**
*@brief		将字符串去掉前/后的特定字符（一般用于去空格）
*@param		pszBuf		[IN]	待转换的字符串，注意：1.调用该函数后pszBuf可能被改变 2.这个参数不一定能返回转换后的字符串
*@param		pszWhat		[IN]	待移除的前/后字符列表，可以指定多个字符，如" \n"则表示移除空格或换行
*@param		nMode		[IN]	移除模式 1去左边 2去右边 3去左右两边
*@return	指向转换后的字符串的指针
*/
char*  PCStrTrim(char *pszBuf, const char *pszWhat = " ", int nMode = 3);

/**
*@brief		将字符串按照pszDelim分割，添加到vecStrList中（如果vecStrList已经有数据，则从最后添加，原来的数据不会被删除）
*@param		pszSrc		[IN]	待分割的字符串.注意：1.调用该函数后pszSrc可能被改变
*@param		pszDelim	[IN]	分割的字符列表，可以指定多个字符，如" \n"则表示按照空格或换行分割
*@param		vecStrList	[OUT]	分割后的字符串列表.注意：此列表仅保存了指针，里面的数据依赖于pszSrc
*@return	错误码，见PC_Lib.h
*/
int  PCStrSplit(char *pszSrc, const char *pszDelim, std::vector<char*>& vecStrList);

/**
*@brief		将命令行参数解析到数组中（如果vecStrList已经有数据，则从最后添加，原来的数据不会被删除）
*@param		argc		[IN]	命令行参数个数
*@param		argv    	[IN]	命令行参数列表
*@param		vecStrList	[OUT]	命令行字符串列表
*@return	错误码，见PC_Lib.h
*/
int  PCCmdArgsParse(int argc, const char* const argv[], std::vector<std::string>& vecStrList);

/**
*@brief		将UTF8编码的字符串进行URL编码
*			参照RFC3986规定，URL编码时默认使用UTF8模式
*@param		pszSrc		[IN]	UTF8编码的待URL编码的原串
*@param		pszDest		[OUT]	URL编码完毕的目的串的缓冲区，返回URL编码后的字符串
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@param		nMode		[IN]	URL编码的模式：
*								nMode = 1 使用js1.5 encodeURI			82个不编码字符!#$&'()*+,/:;=?@-._~0-9a-zA-Z
*								nMode = 2 使用js1.5 encodeURIComponent	71个不编码字符!'()*-._~0-9a-zA-Z
*@return	成功时返回>=0表示目标串pszDest转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCUrlEncode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen, int nMode = 1);

/**
*@brief		将URL编码的字符串进行URL解码，会将+解码为空格，解码后的字符编码依赖于URL编码时的字符编码
*@param		pszSrc		[IN]	待URL解码的原串
*@param		pszDest		[OUT]	URL解码完毕的目的串的缓冲区
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDest转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCUrlDecode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);

/**
*@brief		将GBK编码的字符串转换为UTF8编码
*@param		pszSrc		[IN]	GBK编码的原串
*@param		pszDest		[OUT]	转换为目的串UTF8的缓冲区
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDest转换后实际数据的长度； <时为错误码，见PC_Lib.h
*/
int  PCGbkToUtf8(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);

/**
*@brief		将UTF8编码的字符串转换为GBK编码
*@param		pszSrc		[IN]	UTF8编码的原串
*@param		pszDest		[OUT]	转换为目的串GBK的缓冲区
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDest转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCUtf8ToGbk(const char *pszSrc, char *pszDest, unsigned int nDestBufLen);


/**
*@brief		将两个字节数组异或，结果保存在pszDest中。pszSrc和pszDest的长度最好相等。
*@param		pszSrc		[IN]		需要异或的字符串1
*@param		pszDest		[IN/OUT]	需要异或的字符串2,异或后原来的字符串2将丢失
*@param		nDestBufLen	[IN]		需要异或的字节数
*@return	错误码，见PC_Lib.h
*/
int  PCBytesXor(const unsigned char *pszSrc, unsigned char *pszDest, unsigned int nXorLen);

/**
*@brief		将真实字节数组转换为十六进制显示的字符串，如"123"转换为"313233"
*@param		pszSrc		[IN]	待转换的真实字节数组，GBK编码
*@param		nSrcLen		[IN]	待转换的真实字节数组长度
*@param		pszDestBuf	[OUT]	转换后显示字符串的缓冲区，外部需保证长度足够
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@param		pszDelim	[IN]	转换后每个十六进制表示的字节之间的分隔符(最长8字符)，默认无
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCBytes2DispHexStr(const unsigned char *pszSrc, unsigned int nSrcLen, char* pszDestBuf, unsigned int nDestBufLen, const char *pszDelim = "");

/**
*@brief		将十六进制显示的字符串转换为真实字节数组，如"31 32 33"转换为"123"
*@param		pszSrc		[IN]	待转换的十六进制显示的字符串，任意非"0-9a-fa-A-F"为分隔符
*@param		pszDestBuf	[OUT]	转换后显示字符串的缓冲区，外部需保证长度足够，输出为GBK编码
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCDispHexStr2Bytes(const  char *pszSrc, unsigned char* pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		将字节数组进行压缩，使用GZIP模式 
*@param		pszSrc		[IN]	原串，未压缩的数据
*@param		nSrcLen		[IN]	原串长度
*@param		pszDestBuf	[OUT]	目的串缓冲区，外部需保证长度足够
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCBytesGZipCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		将字节数组进行解压缩，使用GZIP模式
*@param		pszSrc		[IN]	原串，已经压缩好的数据
*@param		nSrcLen		[IN]	原串长度
*@param		pszDestBuf	[OUT]	目的串缓冲区，外部需保证长度足够
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCBytesGZipDeCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen);

/**
*@brief		获取一个随机的无符号正整数，包括0
*@param		
*@return	随机的正整数
*/
unsigned int  PCGetRandomUInt(void);

/**
*@brief		从一个正整数闭区间内随机选取一个正整数，如[3,7]中选出6
*@param		nMin		[IN]	选取的限制最小值，包括自身
*@param		nMax		[IN]	选取的限制最大值，包括自身
*@return	随机选出的正整数.如果nMin>=nMax，则返回nMin.
*/
unsigned int  PCGetRandomRange(unsigned int nMin, unsigned int nMax);

/**
*@brief		获取一个随机的字节数组
*@param		pszDest		[OUT]	字节数组缓冲区
*@param		nDestBufLen	[IN]	需要产生的字节数数量
*@return	错误码，见PC_Lib.h
*/
int  PCGetRandomBytes(unsigned char *pszDest, unsigned int nDestLen);

/**
*@brief		将字节数组进行Base64编码
*@param		pszSrc		[IN]	原串
*@param		nSrcLen		[IN]	原串长度
*@param		pszDestBuf	[OUT]	目的串缓冲区，外部需保证长度足够，分配原串长度两倍空间是比较保险的
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@param		bBlocked	[IN]	是否编码后是分块的(每64字节用换行分块)，默认否
*@param		bUrlSafe	[IN]	Python的url_safe模式，就是将普通base64编码的结果将字符+和/分别变成-和_ ，默认否
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCBase64Encode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked = false, bool bUrlSafe = false);

/**
*@brief		将Base64编码后的数据进行解码
*@param		pszSrc		[IN]	原串
*@param		nSrcLen		[IN]	原串长度
*@param		pszDestBuf	[OUT]	目的串缓冲区，外部需保证长度足够，分配原串长度两倍空间是比较保险的
*@param		nDestBufLen	[IN]	目的串缓冲区长度
*@param		bBlocked	[IN]	原串编码否是分块的(每64字节用换行分块)，默认否
*@param		bUrlSafe	[IN]	Python的url_safe模式，就是编码结果是将普通base64编码的结果将字符+和/分别变成-和_ ，默认否
*@return	成功时返回>=0表示目标串pszDestBuf转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int  PCBase64Decode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked = false, bool bUrlSafe = false);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCSTRINGUTIL_H)
