#ifndef _PCUTILFIREWALLWIN__H_
#define _PCUTILFIREWALLWIN__H_
#if defined(_MSC_VER)
# pragma once
#endif

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

#if defined (_WIN32)

//-------------------------------------------------------------------------------------- 
// UNICODE/ANSI 
//-------------------------------------------------------------------------------------- 
#ifdef UNICODE 
    #define AddApplicationToExceptionList AddApplicationToExceptionListW 
    #define RemoveApplicationFromExceptionList RemoveApplicationFromExceptionListW 
    #define CanLaunchMultiplayerGame CanLaunchMultiplayerGameW 
#else 
	#define AddApplicationToExceptionList AddApplicationToExceptionListA 
	#define RemoveApplicationFromExceptionList RemoveApplicationFromExceptionListA 
	#define CanLaunchMultiplayerGame CanLaunchMultiplayerGameA 
#endif 

#endif	// !defined(_WIN32)

//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////
#endif	// !defined(_PCUTILFIREWALLWIN__H_)
