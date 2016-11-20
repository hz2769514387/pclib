#include "PC_Lib.h"
#include "PCLog.h"
#include "PCUtilSystem.h"
#include "PCUtilMisc_Linux.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////


#if defined(_WIN32)
#else

int LIN_CodeConvert(const char *pszFormCharset, const char *pszToCharset, const char *pszInBuffer, size_t nInBufLen, char *pszOutBuffer, size_t nOutBufLen)
{
	if (pszFormCharset == NULL || pszToCharset == NULL || pszInBuffer == NULL || pszOutBuffer == NULL || nOutBufLen == 0)
	{
		PC_ERROR_LOG("params error!pszFormCharset=%s,pszToCharset=%s,pszInBuffer=%s,nInBufLen=%d,pszOutBuffer=%s,nOutBufLen=%d", pszFormCharset,pszToCharset,pszInBuffer,nInBufLen,pszOutBuffer,nOutBufLen);
		return PC_RESULT_PARAM;
    }

	char **pin = (char **)&pszInBuffer;
	char **pout = &pszOutBuffer;
    size_t sLeftSize = nOutBufLen;

	iconv_t cd = iconv_open(pszToCharset, pszFormCharset);
	if (cd == 0)
	{
		PC_ERROR_LOG("iconv_open = NULL!pszFormCharset=%s,pszToCharset=%s", pszFormCharset,pszToCharset);
		return PC_RESULT_SYSERROR;
	}
    size_t sRet = iconv(cd, pin, &nInBufLen, pout, &sLeftSize);
    if (sRet == (size_t)-1)
	{
		iconv_close(cd);
		PC_ERROR_LOG("iconv = -1!nInBufLen=%d", nInBufLen);
		return PC_RESULT_SYSERROR;
	}
	iconv_close(cd);
	*pout = '\0';
    return (nOutBufLen - sLeftSize);
}

int LIN_EpollEventCtl(int epollFd, int socketFd,  int epctlOp, int events, void * dataPtr)
{
    if(epollFd == PC_INVALID_SOCKET || socketFd == PC_INVALID_SOCKET )
    {
        PC_ERROR_LOG("params error!epollFd=%d,socketFd=%d", epollFd,socketFd);
        return PC_RESULT_PARAM;
    }

    //在执行事件操作
    struct epoll_event epv ;
    memset(&epv, 0, sizeof(epv));
    epv.events = events;
    epv.data.ptr = dataPtr;
    if (0 != epoll_ctl(epollFd, epctlOp, socketFd, &epv))
    {
        PC_ERROR_LOG("epoll_ctl fail.errno = %d, socket fd = %d, op = %d, events = %d", PCGetLastError(), socketFd, epctlOp, events);
        return PC_RESULT_SYSERROR;
    }
    return PC_RESULT_SUCCESS;
}

#endif


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
