#if !defined(_PCSYSTEMUTIL_H_)
#define _PCSYSTEMUTIL_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		获取最后的错误lasterr
*@param		bNetWorkError	[IN]	是否为网络错误码，0：否 其他：是
*@return	最后的错误码lasterrno
*/
int  PCGetLastError(bool bNetWorkError = false);

/**
*@brief		根据系统错误码得到错误描述
*@param		nErrorNo		[IN]	系统错误码
*@param		pszErrMsgBuf	[OUT]	错误描述存放的缓冲区
*@param		nErrMsgBufLen	[IN]	错误描述存放的缓冲区的长度
*@return	错误码，见PC_Lib.h
*/
int  PCGetSysErrorMsg(int nErrorNo, char* pszErrMsgBuf, unsigned int nErrMsgBufLen);

/**
*@brief		将一个全路径分离成目录名和文件名。如将"c:\a\1.txt"分离成"c:\a\"和1.txt
*@param		pszSrcFullPath	[IN]	输入的包括文件名的全路径。路径分隔符支持\和/
*@param		pszPathBuf		[OUT]	目录名存放的缓冲区，最大长度为 PC_MAX_PATH，如果传入NULL，不会输出目录名
*@param		pszPathBuf		[OUT]	文件名存放的缓冲区，最大长度为 PC_MAX_PATH，如果传入NULL，不会输出文件名
*@return	错误码，见PC_Lib.h
*/
int  PCSeperatePath(const char * pszSrcFullPath, char pszPathBuf[PC_MAX_PATH], char pszFileName[PC_MAX_PATH]);

/**
*@brief		获取本模块常用路径，可以指定获取类型
*@param		nPathType		[IN]	路径类型。0.当前工作目录； 1.应用程序路径；
*@param		pszPathBuf		[OUT]	路径存放的缓冲区，最大长度为 PC_MAX_PATH
*@return	错误码，见PC_Lib.h
*/
int  PCGetSelfPath(int nPathType, char pszPathBuf[PC_MAX_PATH]);

/**
*@brief		判断某个资源（如目录、文件等）是否存在
*@param		nResType		[IN]	资源类型。0.目录； 1.文件；
*@param		pszPath			[IN]	资源路径，最大长度为 PC_MAX_PATH
*@return	0函数调用成功且资源不存在；1函数调用成功且资源存在。失败返回<0的错误码，见PC_Lib.h
*/
int  PCResExist(int nResType, const char * pszPath);

/**
*@brief		创建文件夹，可以创建多级文件夹。如果要创建的文件夹已经存在，也返回成功。
*@param		pszPath			[IN]	要创建的文件夹的路径，最大长度为 PC_MAX_PATH
*@return	错误码，见PC_Lib.h
*/
int  PCMkdirRecursive(const char * pszPath);

/**
*@brief		删除文件夹，可以删除多级文件夹和非空文件夹。
*@param		pszPath			[IN]	要删除的文件夹的路径，最大长度为 PC_MAX_PATH
*@return	错误码，见PC_Lib.h
*/
int  PCRemoveRecursive(const char * pszPath);

/**
*@brief		获取文件大小（单位：字节）
*@param		pszPath			[IN]	文件路径，最大长度为 PC_MAX_PATH
*@return	成功时为>=0的文件字节大小，<0时为错误码，见PC_Lib.h
*/
long long PCGetFileSize(const char * pszPath);

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCSYSTEMUTIL_H_)
