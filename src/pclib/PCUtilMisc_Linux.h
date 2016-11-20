#if !defined(_PCUTILMISCLIN_H_)
#define _PCUTILMISCLIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		linux平台独有函数
*/
#if defined(_WIN32)
#else

/**
*@brief		LINUX平台下的编码转换函数
*@param		pszFormCharset	[IN]	输入的数据编码名称，例：utf-8 gb2312
*@param		pszToCharset	[IN]	输出的数据编码名称
*@param		pszInBuffer		[IN]	输入的数据,以lpFormCharset编码。
*@param		nInBufLen		[IN]	输入的数据长度
*@param		pszOutBuffer	[OUT]	输出的数据缓冲区，函数调用结束后的编码为lpToCharset编码。
*@param		nOutBufLen		[IN]	输出的数据缓冲区所分配的空间大小
*@return	成功时返回>=0表示目标串pszOutBuffer转换后实际数据的长度； <0时为错误码，见PC_Lib.h
*/
int LIN_CodeConvert(const char *pszFormCharset, const char *pszToCharset, const char *pszInBuffer, size_t nInBufLen, char *pszOutBuffer, size_t nOutBufLen);

/**
*@brief		LINUX平台下的Epoll注册事件
*@param		epollFd     [IN]	epoll 描述符
*@param		socketFd	[IN]	要注册事件的socket 描述符
*@param		epctlOp		[IN]	要对事件进行的动作，可以是：EPOLL_CTL_ADD EPOLL_CTL_MOD EPOLL_CTL_DEL
*@param		events		[IN]	要提交的事件，可以是以下组合：EPOLLIN EPOLLOUT EPOLLPRI EPOLLERR EPOLLHUP EPOLLET EPOLLONESHOT
*@param		dataPtr     [IN]	事件处理指针
*@return    错误码，见PC_Lib.h
*/
int LIN_EpollEventCtl(int epollFd, int socketFd,  int epctlOp, int events, void * dataPtr);

#endif


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILMISCLIN_H_)
