#include "PC_Lib.h"
#include "PCUtilNetwork.h"

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

int  PCSetNonBlocking(PC_SOCKET  sock)
{
#if defined (_WIN32)
	unsigned long ulArgp = 1;
	int nRet = ioctlsocket(sock, FIONBIO, &ulArgp);
	if (-1 == nRet)
	{
		PC_ERROR_LOG("ioctlsocket fail");
		return PC_RESULT_SYSERROR;
	}
#else
	int opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		PC_ERROR_LOG("fcntl F_GETFL fail");
		return PC_RESULT_SYSERROR;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		PC_ERROR_LOG("fcntl F_SETFL fail");
		return PC_RESULT_SYSERROR;
	}
#endif
	return PC_RESULT_SUCCESS;
}


int PCSockIsConnected(PC_SOCKET sock)
{
	char error[128] = {0};
	socklen_t len = sizeof(int);
#if defined (_WIN32)
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&error, &len) == PC_SOCKET_ERROR)
#else
	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &len) == PC_SOCKET_ERROR)
#endif	
	{
		PC_ERROR_LOG("getsockopt == -1,socket is unconnected");
		return PC_RESULT_SYSERROR;
	}
	return PC_RESULT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
