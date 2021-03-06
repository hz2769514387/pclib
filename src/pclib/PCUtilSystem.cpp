#include "PC_Lib.h"
#include "PCUtilMisc_Linux.h"
#include "PCUtilSystem.h"
#include "PCLog.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

int  PCGetLastError(bool bNetWorkError)
{
#if defined (_WIN32)
	if ( bNetWorkError)
	{
		return WSAGetLastError();
	}
	else
	{
		return GetLastError();
	}
#else
	return errno;
#endif
}

int  PCGetSysErrorMsg(int nErrorNo, char* pszErrMsgBuf, unsigned int nErrMsgBufLen)
{
	if (pszErrMsgBuf == NULL || nErrMsgBufLen == 0)
	{
		PC_ERROR_LOG("param error!nErrorNo=%d,pszErrMsgBuf=%s,nErrMsgBufLen=%u", nErrorNo, pszErrMsgBuf, nErrMsgBufLen);
		return PC_RESULT_PARAM;
	}
	memset(pszErrMsgBuf, 0, nErrMsgBufLen);
	int		nFuncRet = PC_RESULT_SUCCESS;

#if defined (_WIN32)
	LPTSTR	lpMsgBuf = NULL;
	do
	{
		DWORD nFormatLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, nErrorNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
		if (nFormatLen == 0)
		{
			nFuncRet = PC_RESULT_SYSERROR;
			PC_ERROR_LOG("FormatMessage error!nFormatLen = 0,nErrorNo=%d", nErrorNo);
			break;
		}
		if (nFormatLen >= nErrMsgBufLen)
		{
			nFuncRet = PC_RESULT_BUFFNOT_ENOUGH;
			PC_ERROR_LOG("FormatMessage error!nFormatLen(%lu) >= nErrMsgBufLen(%u)", nFormatLen, nErrMsgBufLen);
			break;
		}
		//移除末尾可能出现的回车换行符
		if (lpMsgBuf[nFormatLen - 2] == '\r' && lpMsgBuf[nFormatLen - 1] == '\n')
		{
			memcpy(pszErrMsgBuf, lpMsgBuf, nFormatLen - 2);
			pszErrMsgBuf[nFormatLen - 2] = '\0';
		}
		else
		{
			memcpy(pszErrMsgBuf, lpMsgBuf, nFormatLen);
			pszErrMsgBuf[nFormatLen] = '\0';
		}

		nFuncRet = PC_RESULT_SUCCESS;
	} while (false);
	
	if (lpMsgBuf)
	{
		LocalFree(lpMsgBuf);
		lpMsgBuf = NULL;
	}
#else
	char * pErrMsg = strerror_r(nErrorNo,pszErrMsgBuf,nErrMsgBufLen) ;
	if (pErrMsg == NULL)
	{
		nFuncRet = PC_RESULT_SYSERROR;
		PC_ERROR_LOG("strerror_r error!pErrMsg == NULL");
	}
	else
	{
		strcpy(pszErrMsgBuf, pErrMsg);
		nFuncRet = PC_RESULT_SUCCESS;
	}
#endif
	return nFuncRet;
}

int  PCSeperatePath(const char * pszSrcFullPath, char pszPathBuf[PC_MAX_PATH], char pszFileName[PC_MAX_PATH])
{
	//参数检查
	if (pszSrcFullPath == NULL || pszPathBuf == pszFileName)
	{
		PC_ERROR_LOG("param error!pszSrcFullPath=%s,pszPathBuf=%s,pszFileName=%s", pszSrcFullPath, pszPathBuf, pszFileName);
		return PC_RESULT_PARAM;
	}
	int nSrcFullPathLen = static_cast<int>( strlen(pszSrcFullPath));
	if (nSrcFullPathLen < 2 || nSrcFullPathLen > PC_MAX_PATH || pszSrcFullPath[nSrcFullPathLen - 1] == '\\' || pszSrcFullPath[nSrcFullPathLen - 1] == '/')
	{
		PC_ERROR_LOG("param error!nSrcFullPathLen=%d,pszSrcFullPath=%s", nSrcFullPathLen, pszSrcFullPath);
		return PC_RESULT_PARAM;
	}
	if (pszPathBuf != NULL)
	{
		memset(pszPathBuf, 0, PC_MAX_PATH);
	}
	if (pszFileName != NULL)
	{
		memset(pszFileName, 0, PC_MAX_PATH);
	}
	
	//从后到前找分隔符
	for (int i = nSrcFullPathLen - 1; i >= 0; i--)
	{
		if (pszSrcFullPath[i] == '\\' || pszSrcFullPath[i] == '/')
		{
			if (pszPathBuf != NULL)
			{
				memcpy(pszPathBuf, pszSrcFullPath, i + 1);
				pszPathBuf[i + 1] = '\0';
			}
			if (pszFileName != NULL)
			{
				memcpy(pszFileName, pszSrcFullPath + i + 1, nSrcFullPathLen - i);
				pszFileName[nSrcFullPathLen - i] = '\0';
			}
			return PC_RESULT_SUCCESS;
		}
	}

	PC_ERROR_LOG("tag not find!pszSrcFullPath=%s", pszSrcFullPath);
	return PC_RESULT_FORMATERROR;
}

int  PCGetSelfPath(int nPathType, char pszPathBuf[PC_MAX_PATH])
{
	//参数检查
	if (nPathType < 0 || nPathType > 1 || pszPathBuf == NULL)
	{
		PC_ERROR_LOG("param error!nPathType=%d,pszPathBuf=%s", nPathType, pszPathBuf);
		return PC_RESULT_PARAM;
	}
	memset(pszPathBuf, 0, PC_MAX_PATH);

	if(nPathType == 0)
	{
		//获取工作目录
		if (NULL == PCGetCwd(pszPathBuf, PC_MAX_PATH - 1))
		{
			PC_ERROR_LOG("MyGetCwd = NULL!nPathType=%d,pszPathBuf=%s", nPathType, pszPathBuf);
			return PC_RESULT_SYSERROR;
		}
		return PC_RESULT_SUCCESS;
	}
	else
	{
		//获取app所在目录
		char szAppPath[PC_MAX_PATH] = { 0 };
#if defined (_WIN32)
		if (0 == GetModuleFileNameA(GetModuleHandleA(NULL),szAppPath, PC_MAX_PATH - 1))
#else
		if (-1 == readlink("/proc/self/exe", szAppPath, PC_MAX_PATH - 1) )
#endif
		{
			PC_ERROR_LOG("GetModuleFileName or readlink fail!nPathType=%d,pszPathBuf=%s", nPathType, pszPathBuf);
			return PC_RESULT_SYSERROR;
		}

		return PCSeperatePath(szAppPath, pszPathBuf, NULL);
	}
}

int  PCResExist(int nResType, const char * pszPath)
{
	//参数检查
	if (nResType < 0 || nResType > 1 || pszPath == NULL || strlen(pszPath) == 0 || strlen(pszPath) > PC_MAX_PATH)
	{
		return 0;
	}

	if (nResType == 0)
	{
		//判断目录是否存在
#if defined (_WIN32)
		DWORD dwFileAttr = GetFileAttributesA(pszPath);
		if (INVALID_FILE_ATTRIBUTES == dwFileAttr || !(dwFileAttr&FILE_ATTRIBUTE_DIRECTORY))
		{
			return 0;
		}
		return 1;
#else
		struct stat struStats;
		if (lstat(pszPath, &struStats) == 0 && S_ISDIR(struStats.st_mode))
		{
			return 1;
		}
		return 0;
#endif
	}
	else
	{
		//判断文件是否存在
#if defined (_WIN32)
		DWORD dwFileAttr = GetFileAttributesA(pszPath);
		if (INVALID_FILE_ATTRIBUTES == dwFileAttr || dwFileAttr&FILE_ATTRIBUTE_DIRECTORY)
		{
			return 0;
		}
		return 1;
#else
		struct stat struStats;
		if (lstat(pszPath, &struStats) != 0 || S_ISDIR(struStats.st_mode) )
		{
			return 0;
		}
		return 1;
#endif
	}
}

int  PCMkdirRecursive(const char * pszPath)
{
	//参数检查
	if ( pszPath == NULL || pszPath[0] == 0 || strlen(pszPath) > PC_MAX_PATH)
	{
		return PC_RESULT_PARAM;
	}
	size_t nPathLen = strlen(pszPath);
#if defined (_WIN32)
#else
	//如果是linux，不支持windows形式的盘符命名方式
	if (nPathLen > 1 && pszPath[1] == ':')
	{
		return PC_RESULT_PARAM;
	}
#endif

	// 创建目录  
	char  pszCheckPath[PC_MAX_PATH + 2] = { 0 };
	for (size_t i = 0; i < nPathLen; i++)
	{
		if (pszPath[i] == '\\' || pszPath[i] == '/' || i == (nPathLen-1))
		{
			memset(pszCheckPath, 0, PC_MAX_PATH + 2);
			memcpy(pszCheckPath, pszPath, i + 1);
			pszCheckPath[i + 1] = '\0';
			
			//如果不存在,创建  
			int nRet = PCResExist(0, pszCheckPath);
			if (nRet < 0)
			{
				return nRet;
			}
			else if (nRet == 0)
			{
				nRet = PCMakeDir(pszCheckPath);
				if (nRet != 0)
				{
					return PC_RESULT_SYSERROR;
				}
			}
		}
	}

	return PC_RESULT_SUCCESS;
}


int  PCRemoveRecursive(const char * pszPath)
{
	//参数检查
	if (pszPath == NULL || pszPath[0] == 0 )
	{
		PC_ERROR_LOG("param error! pszPath=%s", pszPath);
		return PC_RESULT_PARAM;
	}

#if defined (_WIN32)
	std::string strDelCmd = "rmdir /S /Q ";
	strDelCmd += pszPath;
#else
	std::string strDelCmd = "rm -rf ";
	strDelCmd += pszPath;
#endif

	if (0 != system(strDelCmd.c_str()))
	{
		PC_ERROR_LOG("pszPath(%s) fail!", pszPath);
		return PC_RESULT_SYSERROR;
	}
	return PC_RESULT_SUCCESS;
}

long long PCGetFileSize(const char * pszPath)
{
#if defined (_WIN32)
	HANDLE hFile = CreateFileA(pszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		PC_ERROR_LOG("CreateFileA(%s) = NULL! errno = %d", pszPath, PCGetLastError());
		return PC_RESULT_FILEOPEN;
	}

	LARGE_INTEGER FileSize; 
	BOOL bRet = GetFileSizeEx(hFile, &FileSize);
	CloseHandle(hFile);
	if (FALSE == bRet)
	{
		PC_ERROR_LOG("GetFileSizeEx(%s) = NULL! errno = %d", pszPath, PCGetLastError());
		return PC_RESULT_SYSERROR;
	}
	return FileSize.QuadPart;

#else
	//打开文件
	FILE* fp = fopen(pszPath, "rb");
	if (NULL == fp)
	{
		PC_ERROR_LOG("fopen(%s) = NULL! errno = %d", pszPath, PCGetLastError());
		return PC_RESULT_FILEOPEN;
	}
	std::shared_ptr<FILE> fAgent(fp, fclose);

	int nRet = fseek(fp, 0, SEEK_END);
	if (0 != nRet)
	{
		PC_ERROR_LOG("fseek(%s) = %d fail! errno = %d", pszPath, nRet, PCGetLastError());
		return PC_RESULT_SYSERROR;
	}
	fpos_t pos;
	nRet = fgetpos(fp, &pos);
	if (0 != nRet)
	{
		PC_ERROR_LOG("fgetpos(%s) = %d fail! errno = %d", pszPath, nRet, PCGetLastError());
		return PC_RESULT_SYSERROR;
	}
	return pos.__pos;
#endif
}


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////