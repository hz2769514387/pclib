#if !defined(_PCUTILNETWORK_H_)
#define _PCUTILNETWORK_H_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

/**
*@brief		设置SOCKET连接为非阻塞模式
*@param		sock	[IN]	套接字
*@return	成功为0，失败为错误码，见PC_Lib.h
*/
int  PCSetNonBlocking(PC_SOCKET sock);

/**
*@brief		判断SOCKET连接是否已连接上
*@param		sock	[IN]	套接字
*@return	成功连接上为0，失败为错误码，见PC_Lib.h
*/
int  PCSockIsConnected(PC_SOCKET sock);


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif // !defined(_PCUTILNETWORK_H_)
