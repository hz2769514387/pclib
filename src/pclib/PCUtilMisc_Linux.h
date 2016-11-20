#if !defined(_PCUTILMISCLIN_H_)
#define _PCUTILMISCLIN_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		linuxƽ̨���к���
*/
#if defined(_WIN32)
#else

/**
*@brief		LINUXƽ̨�µı���ת������
*@param		pszFormCharset	[IN]	��������ݱ������ƣ�����utf-8 gb2312
*@param		pszToCharset	[IN]	��������ݱ�������
*@param		pszInBuffer		[IN]	���������,��lpFormCharset���롣
*@param		nInBufLen		[IN]	��������ݳ���
*@param		pszOutBuffer	[OUT]	��������ݻ��������������ý�����ı���ΪlpToCharset���롣
*@param		nOutBufLen		[IN]	��������ݻ�����������Ŀռ��С
*@return	�ɹ�ʱ����>=0��ʾĿ�괮pszOutBufferת����ʵ�����ݵĳ��ȣ� <0ʱΪ�����룬��PC_Lib.h
*/
int LIN_CodeConvert(const char *pszFormCharset, const char *pszToCharset, const char *pszInBuffer, size_t nInBufLen, char *pszOutBuffer, size_t nOutBufLen);

/**
*@brief		LINUXƽ̨�µ�Epollע���¼�
*@param		epollFd     [IN]	epoll ������
*@param		socketFd	[IN]	Ҫע���¼���socket ������
*@param		epctlOp		[IN]	Ҫ���¼����еĶ����������ǣ�EPOLL_CTL_ADD EPOLL_CTL_MOD EPOLL_CTL_DEL
*@param		events		[IN]	Ҫ�ύ���¼���������������ϣ�EPOLLIN EPOLLOUT EPOLLPRI EPOLLERR EPOLLHUP EPOLLET EPOLLONESHOT
*@param		dataPtr     [IN]	�¼�����ָ��
*@return    �����룬��PC_Lib.h
*/
int LIN_EpollEventCtl(int epollFd, int socketFd,  int epctlOp, int events, void * dataPtr);

#endif


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILMISCLIN_H_)
