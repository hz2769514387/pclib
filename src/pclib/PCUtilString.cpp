#include "PC_Lib.h"
#include "PCUtilMisc_Linux.h"
#include "PCTimeValue.h"
#include "PCUtilString.h"
#include "PCLog.h"


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

char*  PCStrToLower(char *pszBuf)
{
	if (NULL == pszBuf)
	{
		return pszBuf;
	}

	char *p = pszBuf;
	while (*p) 
	{
		//不转换扩展ASCII字符，防止中文变乱码
		if ((*p) & 0x80)
		{
			//如果某一个字节为扩展ASCII开头，则接着的一个字节被忽略
			p++;
		}
		else if ((*p) >= 'A' && (*p) <= 'Z')
		{
			(*p) += 32;
		}
		p++;
	}
	return pszBuf;
}

char*  PCStrToUpper(char *pszBuf)
{
	if (NULL == pszBuf)
	{
		return pszBuf;
	}

	char *p = pszBuf;
	while (*p)
	{
		if ((*p) & 0x80)
		{
			p++;
		}
		else if ((*p) >= 'a' && (*p) <= 'z')
		{
			(*p) -= 32;
		}
		p++;
	}
	return pszBuf;
}

const char* PCStrStricase(const char* str1, const char* str2)
{
	if (str1 == NULL || str1[0] == 0 || str2 == NULL)
	{
		return NULL;
	}

	size_t lenSub = strlen(str2);
	if (lenSub == 0)
	{
		return NULL;
	}

	while (*str1)
	{
		if (PCStrnCaseCmp(str1, str2, lenSub) == 0)
		{
			return str1;
		}
		++str1;
	}
	return NULL;
}

char*  PCStrTrim(char *pszBuf, const char *pszWhat, int nMode)
{
	if (NULL == pszBuf || NULL == pszWhat || nMode < 1 || nMode > 3)
	{
		PC_ERROR_LOG("params err! pszBuf=%s, pszWhat=%s, nMode=%d", pszBuf, pszWhat, nMode);
		return pszBuf;
	}

	unsigned char *what = (unsigned char *)pszWhat;
	unsigned char *buf = (unsigned char *)pszBuf;
	
	char mask[0xFF];
	memset(mask, 0, 0xFF);
	while (*what) 
	{
		mask[*what] = '\1';
		what++;
	}
	if (nMode & 1) 
	{ 
		// 前面
		what = buf;
		while (*what) 
		{
			if (!mask[*what]) 
			{
				break;
			}
			buf++;
			what++;
		}
	}
	if (nMode & 2)
	{
		// 后面
		what = buf + strlen((const char*)buf) - 1;
		while (what >= buf)
		{
			if (!mask[*what]) 
			{
				break;
			}
			what--;
		}
		what++;
		*what = '\0';
	}
	return (char*)buf;
}

int PCStrSplit(char *pszSrc, const char *pszDelim, std::vector<char*> &vecStrList)
{
	if (pszSrc == NULL)
	{
		PC_ERROR_LOG("param error！pszSrc=NULL");
		return PC_RESULT_PARAM;
	}

	if (pszDelim == NULL)
	{
		vecStrList.push_back(pszSrc);
		return PC_RESULT_SUCCESS;
	}

	const char *spanp;
	char       *s = pszSrc;
	while (*s)
	{
		spanp = pszDelim;
		while (*spanp)
		{
			if (*s == *spanp)
			{
				*s = '\0';
				if (pszSrc[0] != 0)
				{
					vecStrList.push_back(pszSrc);
				}
				pszSrc = s + 1;
				break;
			}
			spanp++;
		}
		s++;
	}
	if (*pszSrc && pszSrc[0] != 0)
	{
		vecStrList.push_back(pszSrc);
	}
	return PC_RESULT_SUCCESS;
}

int  PCCmdArgsParse(int argc, const char* const argv[], std::vector<std::string>& vecStrList)
{
	if (argc < 1 || argv[0] == 0)
	{
		PC_ERROR_LOG("param error！argc=%d", argc);
		return PC_RESULT_PARAM;
	}

	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == NULL || argv[i][0] == 0)
		{
			PC_ERROR_LOG("param error！argc=%d, argv[%d] is empty", argc, i);
			return PC_RESULT_PARAM;
		}
		std::string strParam = argv[i];
		vecStrList.push_back(strParam);
	}
	return PC_RESULT_SUCCESS;
}

int  PCUrlEncode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen, int nMode)
{
	//参数检查
	if (pszSrc == NULL || pszDest == NULL || nDestBufLen == 0 || nMode < 1 || nMode > 2)
	{
		PC_ERROR_LOG("param error！pszSrc=%s,pszDest=%s,nDestBufLen=%u,nMode=%d", pszSrc, pszDest, nDestBufLen, nMode);
		return PC_RESULT_PARAM;
	}
	if (pszSrc[0] == 0)
	{
		return 0;
	}
	memset(pszDest, 0, nDestBufLen);

	//MASK没必要每次都计算，这是encodeURI计算过程示例
	/*char * what = "!#$&'()*+,/:;=?@-._~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char mask[0xFF];
	memset(mask, 0, 0xFF);
	while (*what)
	{
	mask[*what] = '\1';
	what++;
	}*/
	//MASK计算结果
	static const unsigned char MASK_ENCODE_URI[0xFF] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	static const unsigned char MASK_ENCODE_URLCOMPONENT[0xFF] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\1\1\1\1\0\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	static const unsigned char BASE_CHAR = 'A' ;	//如果要更改编码格式为小写如%cf模式,则baseChar需更改为'a'
	unsigned int nDestDataLen = 0;
	for (; *pszSrc; pszSrc++)
	{
		if (nDestDataLen >= nDestBufLen - 4)
		{
			PC_ERROR_LOG("buf too short！nDestDataLen(%u) >= nDestBufLen(%u) - 4", nDestDataLen, nDestBufLen);
			return PC_RESULT_BUFFNOT_ENOUGH;
		}

		unsigned char c = *pszSrc;
		if ((nMode == 1 && (!MASK_ENCODE_URI[c])) || (nMode == 2 && (!MASK_ENCODE_URLCOMPONENT[c])))
		{
			pszDest[0] = '%';
			pszDest[1] = (c >= 0xA0) ? ((c >> 4) - 10 + BASE_CHAR) : ((c >> 4) + '0');
			pszDest[2] = ((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + BASE_CHAR) : ((c & 0xF) + '0');
			pszDest			+= 3;
			nDestDataLen	+= 3;
		}
		else
		{
			pszDest[0] = c;
			pszDest += 1;
			nDestDataLen += 1;
		}
	}
	*pszDest = '\0';
	return nDestDataLen;
}

int  PCUrlDecode(const char *pszSrc, char *pszDest, unsigned int nDestBufLen)
{
	//参数检查
	if (pszSrc == NULL || pszDest == NULL || nDestBufLen == 0)
	{
		PC_ERROR_LOG("param error！pszSrc=%s,pszDest=%s,nDestBufLen=%u", pszSrc, pszDest, nDestBufLen);
		return PC_RESULT_PARAM;
	}
	memset(pszDest, 0, nDestBufLen);
	unsigned int nSrcLen = (int)strlen(pszSrc);
	if (nSrcLen == 0)
	{
		return 0;
	}
	if (nSrcLen >= nDestBufLen)
	{
		PC_ERROR_LOG("buf too short！nSrcLen(%u) >= nDestBufLen(%u)", nSrcLen, nDestBufLen);
		return PC_RESULT_BUFFNOT_ENOUGH;
	}

	int nDestDataLen = 0;
	for (; *pszSrc; pszSrc++, pszDest++, nDestDataLen++)
	{
		if (*pszSrc == '+') 
		{
			*pszDest = ' ';
		}
		else if (*pszSrc == '%' && isxdigit(*(pszSrc + 1)) && isxdigit(*(pszSrc + 2)))
		{
			int c = 0;
			for (int i = 1; i <= 2; i++) 
			{
				c <<= 4;
				if (pszSrc[i] >= '0' && pszSrc[i] <= '9')
				{
					c |= (pszSrc[i] - '0');
				}
				else if (pszSrc[i] >= 'a' && pszSrc[i] <= 'f')
				{
					c |= (pszSrc[i] - 'a') + 10;
				}
				else if (pszSrc[i] >= 'A' && pszSrc[i] <= 'F')
				{
					c |= (pszSrc[i] - 'A') + 10;
				}
			}
			*pszDest = (char)(c & 0xff);
			pszSrc += 2;
		}
		else 
		{
			*pszDest = *pszSrc;
		}
	}
	*pszDest = '\0';
	return nDestDataLen;
}

int PCGbkToUtf8(const char *pszSrc, char *pszDest, unsigned int  nDestBufLen)
{
	//参数检查
	if (pszSrc == NULL || pszDest == NULL || nDestBufLen == 0)
	{
		PC_ERROR_LOG("param error！pszSrc=%s,pszDest=%s,nDestBufLen=%u", pszSrc, pszDest, nDestBufLen);
		return PC_RESULT_PARAM;
	}
	memset(pszDest, 0, nDestBufLen);
	int nSrcLen = (int)strlen(pszSrc);
	if (nSrcLen == 0)
	{
		return 0;
	}

#if defined (_WIN32)
	//gbk-unicode  
	int nWcsLen = MultiByteToWideChar(CP_ACP, NULL, pszSrc, nSrcLen, NULL, 0);
	if (nWcsLen == 0)
	{
		PC_ERROR_LOG("MultiByteToWideChar nWcsLen = 0！");
		return PC_RESULT_SYSERROR;
	}
	std::unique_ptr<wchar_t[]> upUnicodeData(new wchar_t[nWcsLen + 1]);
	if (upUnicodeData.get() == NULL)
	{
		PC_ERROR_LOG("new wchar_t pData = NULL");
		return PC_RESULT_MEMORY;
	}
	if (0 == MultiByteToWideChar(CP_ACP, NULL, pszSrc, nSrcLen, upUnicodeData.get(), nWcsLen))
	{
		PC_ERROR_LOG("MultiByteToWideChar ret = 0！");
		return PC_RESULT_SYSERROR;
	}
	upUnicodeData.get()[nWcsLen] = '\0';

	//unicode-utf8
	int nUtf8Len = WideCharToMultiByte(CP_UTF8, NULL, upUnicodeData.get(), nWcsLen, NULL, 0, NULL, NULL);
	if (nUtf8Len <= 0)
	{
		PC_ERROR_LOG("WideCharToMultiByte nUtf8Len = 0！");
		return PC_RESULT_SYSERROR;
	}
	if ((unsigned int)nUtf8Len > nDestBufLen)
	{
		PC_ERROR_LOG("nUtf8Len(%d) > nDestBufLen(%u)！", nUtf8Len, nDestBufLen);
		return PC_RESULT_BUFFNOT_ENOUGH;
	}
	if (0 == WideCharToMultiByte(CP_UTF8, NULL, upUnicodeData.get(), nWcsLen, pszDest, nUtf8Len, NULL, NULL))
	{
		PC_ERROR_LOG("WideCharToMultiByte ret = 0！");
		return PC_RESULT_SYSERROR;
	}
	pszDest[nUtf8Len] = '\0';
	return nUtf8Len;
#else
	return LIN_CodeConvert("gb2312", "utf-8", pszSrc, nSrcLen, pszDest, nDestBufLen);
#endif
}

int PCUtf8ToGbk(const char *pszSrc, char *pszDest, unsigned int  nDestBufLen)
{
	//参数检查
	if (pszSrc == NULL || pszDest == NULL || nDestBufLen <= 0)
	{
		PC_ERROR_LOG("param error！pszSrc=%s,pszDest=%s,nDestBufLen=%u", pszSrc, pszDest, nDestBufLen);
		return PC_RESULT_PARAM;
	}
	memset(pszDest, 0, nDestBufLen);
	int nSrcLen = (int)strlen(pszSrc);
	if (nSrcLen == 0)
	{
		return 0;
	}

#if defined (_WIN32)
	//utf8-unicode  
	int nWcsLen = MultiByteToWideChar(CP_UTF8, NULL, pszSrc, nSrcLen, NULL, 0);
	if (nWcsLen == 0)
	{
		PC_ERROR_LOG("MultiByteToWideChar nWcsLen = 0！");
		return PC_RESULT_SYSERROR;
	}
	std::unique_ptr<wchar_t[]> upUnicodeData(new wchar_t[nWcsLen + 1]);
	if (upUnicodeData.get() == NULL)
	{
		PC_ERROR_LOG("new wchar_t pData = NULL！");
		return PC_RESULT_MEMORY;
	}
	if (0 == MultiByteToWideChar(CP_UTF8, NULL, pszSrc, nSrcLen, upUnicodeData.get(), nWcsLen))
	{
		PC_ERROR_LOG("MultiByteToWideChar ret = 0！");
		return PC_RESULT_SYSERROR;
	}
	upUnicodeData.get()[nWcsLen] = '\0';

	//unicode-gbk/ansi
	int nGbkLen = WideCharToMultiByte(CP_ACP, NULL, upUnicodeData.get(), nWcsLen, NULL, 0, NULL, NULL);
	if ( nGbkLen <= 0 )
	{
		PC_ERROR_LOG("WideCharToMultiByte nGbkLen = 0！");
		return PC_RESULT_SYSERROR;
	}
	if ((unsigned int)nGbkLen > nDestBufLen)
	{
		PC_ERROR_LOG("nGbkLen(%d) > nDestBufLen(%u)！", nGbkLen, nDestBufLen);
		return PC_RESULT_BUFFNOT_ENOUGH;
	}
	if (0 == WideCharToMultiByte(CP_ACP, NULL, upUnicodeData.get(), nWcsLen, pszDest, nGbkLen, NULL, NULL))
	{
		PC_ERROR_LOG("WideCharToMultiByte ret = 0！");
		return PC_RESULT_SYSERROR;
	}
	pszDest[nGbkLen] = '\0';
	return nGbkLen;
#else
	return LIN_CodeConvert("utf-8", "gb2312", pszSrc, nSrcLen, pszDest, nDestBufLen);
#endif
}

int  PCMbsToUnicode(const char *pszSrc, std::vector<wchar_t> &pszDest, bool bUtf8 )
{
	//参数检查
	if (pszSrc == NULL )
	{
		PC_ERROR_LOG("param error！pszSrc=NULL");
		return PC_RESULT_PARAM;
	}
	pszDest.clear();

#if defined (_WIN32)
	unsigned int codePage = bUtf8 ? CP_UTF8 : CP_ACP;
	
	int nWcsLen = MultiByteToWideChar(codePage, NULL, pszSrc, -1, NULL, 0);
	if (nWcsLen == 0)
	{
		PC_ERROR_LOG("MultiByteToWideChar nWcsLen = 0！");
		return PC_RESULT_SYSERROR;
	}
	pszDest.resize(nWcsLen,'\0');
	
	if (0 == MultiByteToWideChar(codePage, NULL, pszSrc, -1, &pszDest[0], nWcsLen))
	{
		PC_ERROR_LOG("MultiByteToWideChar ret = 0！");
		return PC_RESULT_SYSERROR;
	}
#else
	if (bUtf8)
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.utf8"),"setlocale utf8 fail");
	}
	else
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
	}

	int nWcsLen = mbstowcs(NULL, pszSrc, 0) ;
	if (-1 == nWcsLen)
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
		PC_ERROR_LOG("mbstowcs nWcsLen = -1！");
		return PC_RESULT_SYSERROR;
	}
	pszDest.resize(nWcsLen+1, '\0');

	if (-1 == mbstowcs(&pszDest[0], pszSrc, nWcsLen + 1))
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
		PC_ERROR_LOG("mbstowcs ret = -1！");
		return PC_RESULT_SYSERROR;
	}
	PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"), "setlocale gbk fail");
#endif
	
	return PC_RESULT_SUCCESS;
}

int  PCUnicodeToMbs(const wchar_t *pszSrc, std::vector<char> &pszDest, bool bUtf8)
{
	//参数检查
	if (pszSrc == NULL)
	{
		PC_ERROR_LOG("param error！pszSrc=NULL");
		return PC_RESULT_PARAM;
	}
	pszDest.clear();

#if defined (_WIN32)
	unsigned int codePage = bUtf8 ? CP_UTF8 : CP_ACP;

	int nMbsLen = WideCharToMultiByte(codePage, NULL, pszSrc, -1, NULL, 0, NULL, NULL);
	if (nMbsLen <= 0)
	{
		PC_ERROR_LOG("WideCharToMultiByte nGbkLen = 0！");
		return PC_RESULT_SYSERROR;
	}
	pszDest.resize(nMbsLen, '\0');

	if (0 == WideCharToMultiByte(codePage, NULL, pszSrc, -1, &pszDest[0], nMbsLen, NULL, NULL))
	{
		PC_ERROR_LOG("WideCharToMultiByte ret = 0！");
		return PC_RESULT_SYSERROR;
	}
#else
	if (bUtf8)
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.utf8"),"setlocale utf8 fail");
	}
	else
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
	}

	int nMbsLen =  wcstombs(NULL, pszSrc, 0);
	if (-1 == nMbsLen)
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
		PC_ERROR_LOG("mbstowcs nWcsLen = -1！");
		return PC_RESULT_SYSERROR;
	}
	pszDest.resize(nMbsLen + 1, '\0');

	if (-1 ==  wcstombs(&pszDest[0], pszSrc, nMbsLen + 1))
	{
		PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"),"setlocale gbk fail");
		PC_ERROR_LOG("mbstowcs ret = -1！");
		return PC_RESULT_SYSERROR;
	}
	PC_ASSERT(setlocale(LC_CTYPE, "zh_CN.gbk"), "setlocale gbk fail");
#endif

	return PC_RESULT_SUCCESS;
}



int   PCBytesXor(const unsigned char *pszSrc, unsigned char *pszDest, unsigned int nXorLen)
{
	if (pszSrc == NULL || pszDest == NULL )
	{
		PC_ERROR_LOG("params err!  pszDest=%s, nXorLen=%u",  pszDest, nXorLen);
		return PC_RESULT_PARAM;
	}
	for (unsigned int i = 0; i < nXorLen; i++)
	{
		pszDest[i] ^= pszSrc[i];
	}
	return PC_RESULT_SUCCESS;
}

int  PCBytes2DispHexStr(const unsigned char *pszSrc, unsigned int nSrcLen, char* pszDestBuf, unsigned int nDestBufLen, const char *pszDelim)
{
	size_t nDelimStrLen = pszDelim ? strlen(pszDelim) : 0;
	if (pszSrc == NULL || nSrcLen == 0 || pszDestBuf == NULL || nDestBufLen <= ((2 + nDelimStrLen) *nSrcLen) || pszDelim == NULL || nDelimStrLen > 8)
	{
		PC_ERROR_LOG("params err!  nSrcLen = %u, nDestBufLen=%u, pszDelim=%s", nSrcLen, nDestBufLen,pszDelim);
		return PC_RESULT_PARAM;
	}

	size_t nDestDataLen = 0;
	char pszTmpBuf[12];
	for (unsigned int i = 0; i < nSrcLen; i++)
	{
		if (i < nSrcLen - 1)
		{
			if (pszSrc[i] == 0)
			{
				pszTmpBuf[0] = '0';
				pszTmpBuf[1] = '0';
				memcpy(pszTmpBuf + 2, pszDelim, nDelimStrLen);
			}
			else
			{
				sprintf(pszTmpBuf, "%02X%s", pszSrc[i], pszDelim);
			}

			memcpy(pszDestBuf + nDestDataLen, pszTmpBuf, 2 + nDelimStrLen);
			nDestDataLen += (2 + nDelimStrLen);
		}
		else
		{
			if (pszSrc[i] == 0)
			{
				pszTmpBuf[0] = '0';
				pszTmpBuf[1] = '0';
			}
			else
			{
				sprintf(pszTmpBuf, "%02X", pszSrc[i]);
			}

			memcpy(pszDestBuf + nDestDataLen, pszTmpBuf, 2);
			nDestDataLen += 2;
		}
	}
	pszDestBuf[nDestDataLen] = '\0';
	return ((int)nDestDataLen);
}

int  PCDispHexStr2Bytes(const  char *pszSrc, unsigned char* pszDestBuf, unsigned int nDestBufLen)
{
	if (pszSrc == NULL || pszDestBuf == NULL || nDestBufLen == 0)
	{
		PC_ERROR_LOG("params err! pszSrc=%s, nDestBufLen=%u", pszSrc, nDestBufLen);
		return PC_RESULT_PARAM;
	}

	unsigned int nDestDataLen = 0;
	int sTmpVal = 0;
	unsigned char buf[3];
	for (; *pszSrc; pszSrc++)
	{
		if (isxdigit(*pszSrc))
		{
			if (!isxdigit(*(pszSrc + 1)))
			{
				PC_ERROR_LOG("format err! pszSrc=%s", pszSrc);
				return PC_RESULT_FORMATERROR;
			}
			if (nDestDataLen >= nDestBufLen)
			{
				PC_ERROR_LOG("buf too short! nDestDataLen(%u) >= nDestBufLen(%u)", nDestDataLen, nDestBufLen);
				return PC_RESULT_BUFFNOT_ENOUGH;
			}

			memcpy(buf, pszSrc, 2);
			buf[2] = 0;
			sscanf((char *)buf, "%02X", &sTmpVal);
			pszDestBuf[nDestDataLen] = (unsigned char)sTmpVal;
			nDestDataLen++;
			pszSrc++;
		}
	}
	pszDestBuf[nDestDataLen] = '\0';
	return nDestDataLen;
}

int  PCBytesGZipCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszDestBuf == NULL || nDestBufLen == 0)
	{
		PC_ERROR_LOG("params err!  nSrcLen = %d, nDestBufLen=%d",  nSrcLen, nDestBufLen);
		return PC_RESULT_PARAM;
	}
	int nRet = 0;

	z_stream c_stream;
	c_stream.zalloc = NULL;
	c_stream.zfree = NULL;
	c_stream.opaque = NULL;

	//只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
	nRet = deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
	if (nRet != Z_OK)
	{
		PC_ERROR_LOG("deflateInit2 err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
		return PC_RESULT_SYSERROR;
	}
	c_stream.next_in = (unsigned char *)pszSrc;
	c_stream.avail_in = nSrcLen;
	c_stream.next_out = pszDestBuf;
	c_stream.avail_out = nDestBufLen;

	while (c_stream.avail_in != 0 ) 
	{
		if (c_stream.total_out >= nDestBufLen)
		{
			PC_ERROR_LOG("buf too short!  c_stream.total_out(%lu) >= nDestBufLen(%ld)", c_stream.total_out, nDestBufLen);
			return PC_RESULT_BUFFNOT_ENOUGH;
		}
		if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
		{
			PC_ERROR_LOG("deflate err!  nSrcLen = %d, nDestBufLen=%d", nSrcLen, nDestBufLen);
			return PC_RESULT_SYSERROR;
		}
	}
	while (true)
	{
		nRet = deflate(&c_stream, Z_FINISH);
		if (nRet == Z_STREAM_END)
		{
			break;
		}
		if (nRet != Z_OK)
		{
			PC_ERROR_LOG("while deflate err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
			return PC_RESULT_SYSERROR;
		}
	}

	nRet = deflateEnd(&c_stream);
	if (nRet != Z_OK)
	{
		PC_ERROR_LOG("deflateEnd err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
		return PC_RESULT_SYSERROR;
	}
	return c_stream.total_out;
}

int  PCBytesGZipDeCompress(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen)
{
	if (pszSrc == NULL || nSrcLen == 0 || pszDestBuf == NULL || nDestBufLen == 0)
	{
		PC_ERROR_LOG("params err!  nSrcLen = %d, nDestBufLen=%d",  nSrcLen, nDestBufLen);
		return PC_RESULT_PARAM;
	}
	int nRet = 0;
	
	static const char dummy_head[2] = 
	{
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};

	z_stream d_stream = { 0 };
	d_stream.zalloc = NULL;
	d_stream.zfree = NULL;
	d_stream.opaque = NULL;

	d_stream.next_in = (unsigned char *)pszSrc;
	d_stream.avail_in = 0;
	d_stream.next_out = pszDestBuf;

	//只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
	//if (inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) return -1;
	nRet = inflateInit2(&d_stream, 47);
	if (nRet != Z_OK)
	{
		PC_ERROR_LOG("inflateInit2 err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
		return PC_RESULT_SYSERROR;
	}

	while (d_stream.total_out < nDestBufLen && d_stream.total_in < nSrcLen)
	{
		/* force small buffers */
		d_stream.avail_in = d_stream.avail_out = 1; 

		nRet = inflate(&d_stream, Z_NO_FLUSH);
		if (nRet == Z_STREAM_END)
		{
			break;
		}
		if (nRet != Z_OK) 
		{
			if (nRet == Z_DATA_ERROR)
			{
				d_stream.next_in = (unsigned char *)dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				nRet = inflate(&d_stream, Z_NO_FLUSH);
				if (nRet == Z_STREAM_END)
				{
					break;
				}
				if (nRet != Z_OK)
				{
					PC_ERROR_LOG("Z_DATA_ERROR inflate err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
					return PC_RESULT_SYSERROR;
				}
			}
			else
			{
				PC_ERROR_LOG("inflate err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
				return PC_RESULT_SYSERROR;
			}
		}
	}

	nRet = inflateEnd(&d_stream);
	if (nRet != Z_OK)
	{
		PC_ERROR_LOG("inflateEnd err!  nSrcLen = %d, nDestBufLen=%d, nRet=%d", nSrcLen, nDestBufLen, nRet);
		return PC_RESULT_SYSERROR;
	}

	return d_stream.total_out;
}

unsigned int  PCGetRandomUInt(void)
{
	static struct PCStruMT19937 m_Mt19937;
	static bool   bInited = false;
	if (!bInited)
	{
		m_Mt19937.srand(static_cast<unsigned int>(CPCTimeValue::TickCount().GetValue()));
		bInited = true;
	}
	return m_Mt19937.rand();
}

unsigned int  PCGetRandomRange(unsigned int nMin, unsigned int nMax)
{
	if (nMin >= nMax)
	{
		return nMin;
	}	
	return (PCGetRandomUInt() % (nMax - nMin + 1) +  nMin);
}

int  PCGetRandomBytes(unsigned char *pszDest, unsigned int nDestLen)
{
	if (pszDest == NULL )
	{
		PC_ERROR_LOG("params err! pszDest=NULL");
		return PC_RESULT_PARAM;
	}

	for (unsigned int i = 0; i < nDestLen; i++)
	{
		pszDest[i] = static_cast<unsigned char>(PCGetRandomUInt() % (0xFF + 1));
	}
	return PC_RESULT_SUCCESS;
}

int  PCBase64Encode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked , bool bUrlSafe )
{
	if (pszSrc == NULL || nSrcLen == 0 || pszDestBuf == NULL || nDestBufLen <= nSrcLen)
	{
		PC_ERROR_LOG("params err!  nSrcLen = %d, nDestBufLen=%d",  nSrcLen, nDestBufLen);
		return PC_RESULT_PARAM;
	}

	//普通base64编码
	int nBase64DestLen = 0;
	if (bBlocked)
	{
		//分块
		EVP_ENCODE_CTX context;
		EVP_EncodeInit(&context);

		int nOutLen = 0;
		EVP_EncodeUpdate(&context, pszDestBuf, &nOutLen, pszSrc, nSrcLen);
		nBase64DestLen += nOutLen;

		EVP_EncodeFinal(&context, pszDestBuf + nOutLen, &nOutLen);
		nBase64DestLen += nOutLen;
	}
	else
	{
		//不分块
		nBase64DestLen = EVP_EncodeBlock(pszDestBuf, pszSrc, nSrcLen);
	}

	//如果为url_safe模式，将字符+和/分别变成-和_ 
	if (bUrlSafe)
	{
		for (int i = 0; i < nBase64DestLen; i++)
		{
			if (pszDestBuf[i] == '+')
			{
				pszDestBuf[i] = '-';
			}
			if (pszDestBuf[i] == '/')
			{
				pszDestBuf[i] = '_';
			}
		}
	}
	return nBase64DestLen;
}

int  PCBase64Decode(const unsigned char *pszSrc, unsigned int nSrcLen, unsigned char *pszDestBuf, unsigned int nDestBufLen, bool bBlocked , bool bUrlSafe )
{
	if (pszSrc == NULL || nSrcLen == 0 || pszDestBuf == NULL || nDestBufLen <= nSrcLen)
	{
		PC_ERROR_LOG("params err!  nSrcLen = %d, nDestBufLen=%d", nSrcLen, nDestBufLen);
		return PC_RESULT_PARAM;
	}

	unsigned char * pRealSrc = (unsigned char*)malloc(sizeof(unsigned char) * nSrcLen + 4);
	if (NULL == pRealSrc)
	{
		PC_ERROR_LOG("malloc err!  nSrcLen = %d, nDestBufLen=%d", nSrcLen, nDestBufLen);
		return PC_RESULT_MEMORY;
	}

	//如果为url_safe模式，将字符的转换还原回来，-和_分别变成 +和/
	if (bUrlSafe)
	{
		for (unsigned int i = 0; i < nSrcLen; i++)
		{
			if (pszSrc[i] == '-')
			{
				pRealSrc[i] = '+';
			}
			else if (pszSrc[i] == '_')
			{
				pRealSrc[i] = '/';
			}
			else
			{
				pRealSrc[i] = pszSrc[i];
			}
		}
	}
	else
	{
		memcpy(pRealSrc, pszSrc, nSrcLen);
	}

	//判断是否bBlocked
	int nDecodedLen = 0;
	if (bBlocked)
	{
		EVP_ENCODE_CTX context;
		EVP_DecodeInit(&context);

		int nOutLen = 0;
		EVP_DecodeUpdate(&context, pszDestBuf, &nOutLen, pRealSrc, nSrcLen);
		nDecodedLen += nOutLen;

		EVP_DecodeFinal(&context, pszDestBuf + nOutLen, &nOutLen);
		nDecodedLen += nOutLen;
	}
	else
	{
		//对非4整数倍的数据做处理
		int nLeft = nSrcLen % 4;
		nSrcLen += nLeft;
		for (int i = 0; i < nLeft; i++)
		{
			pRealSrc[nSrcLen - i - 1] = '=';
		}

		//先把填充的=号数量算出来，解码后再去掉这个长度
		int nPaddingLen = 0;
		for (int i = nSrcLen - 1; i >= 0; i--)
		{
			if (pRealSrc[i] != '=')
			{
				break;
			}
			nPaddingLen++;
		}

		nDecodedLen = EVP_DecodeBlock(pszDestBuf, pRealSrc, nSrcLen);
		nDecodedLen -= nPaddingLen;
	}

	free(pRealSrc);
	return nDecodedLen;
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
