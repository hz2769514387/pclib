#include "PC_Lib.h"
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


#endif


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
