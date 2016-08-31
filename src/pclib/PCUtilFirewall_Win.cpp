//AddApplicationToExceptionList
#include "PC_Lib.h"
#include "PCUtilFirewall_Win.h"
#include "PCLog.h"
//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_BEG
//////////////////////////////////////////////////////////////////////////

#if defined (_WIN32)



//-------------------------------------------------------------------------------------- 
// Get the INetFwProfile interface for current profile 
//-------------------------------------------------------------------------------------- 
INetFwProfile* GetFirewallProfile() 
{ 
    HRESULT hr; 
    INetFwMgr* pFwMgr = NULL; 
    INetFwPolicy* pFwPolicy = NULL; 
    INetFwProfile* pFwProfile = NULL; 
 
    // Create an instance of the Firewall settings manager 
    hr = CoCreateInstance( __uuidof( NetFwMgr ), NULL, CLSCTX_INPROC_SERVER, 
                           __uuidof( INetFwMgr ), ( void** )&pFwMgr ); 
    if( SUCCEEDED( hr ) ) 
    { 
        hr = pFwMgr->get_LocalPolicy( &pFwPolicy ); 
        if( SUCCEEDED( hr ) ) 
        { 
            pFwPolicy->get_CurrentProfile( &pFwProfile ); 
        } 
    } 
 
    // Cleanup 
    if( pFwPolicy ) pFwPolicy->Release(); 
    if( pFwMgr ) pFwMgr->Release(); 
 
    return pFwProfile; 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Adds application from exception list 
//-------------------------------------------------------------------------------------- 
HRESULT AddApplicationToExceptionListW( WCHAR* strGameExeFullPath, WCHAR* strFriendlyAppName ) 
{ 
    HRESULT hr = E_FAIL; 
    bool bCleanupCOM = false; 
    BSTR bstrFriendlyAppName = NULL; 
    BSTR bstrGameExeFullPath = NULL; 
    INetFwAuthorizedApplication* pFwApp = NULL; 
    INetFwAuthorizedApplications* pFwApps = NULL; 
    INetFwProfile* pFwProfile = NULL; 
 
#ifdef SHOW_DEBUG_MSGBOXES 
        WCHAR sz[1024]; 
        StringCchPrintf( sz, 1024, L"strFriendlyAppName='%s' strGameExeFullPath='%s'", strFriendlyAppName, strGameExeFullPath ); 
        MessageBox( NULL, sz, L"AddApplicationToExceptionListW", MB_OK ); 
#endif 
 
    if( strGameExeFullPath == NULL || strFriendlyAppName == NULL ) 
    { 
        PC_ASSERT( false ,"trGameExeFullPath == NULL || strFriendlyAppName == NULL"); 
        return E_INVALIDARG; 
    } 
 
    bstrGameExeFullPath = SysAllocString( strGameExeFullPath ); 
    bstrFriendlyAppName = SysAllocString( strFriendlyAppName ); 
    if( bstrGameExeFullPath == NULL || bstrFriendlyAppName == NULL ) 
    { 
        hr = E_OUTOFMEMORY; 
        goto LCleanup; 
    } 
 
    hr = CoInitialize( 0 ); 
    bCleanupCOM = SUCCEEDED( hr ); 
 
    pFwProfile = GetFirewallProfile(); 
    if( pFwProfile == NULL ) 
    { 
        hr = E_FAIL; 
        goto LCleanup; 
    } 
 
    hr = pFwProfile->get_AuthorizedApplications( &pFwApps ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Create an instance of an authorized application. 
    hr = CoCreateInstance( __uuidof( NetFwAuthorizedApplication ), NULL, 
                           CLSCTX_INPROC_SERVER, __uuidof( INetFwAuthorizedApplication ), ( void** )&pFwApp ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Set the process image file name. 
    hr = pFwApp->put_ProcessImageFileName( bstrGameExeFullPath ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Set the application friendly name. 
    hr = pFwApp->put_Name( bstrFriendlyAppName ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Add the application to the collection. 
    hr = pFwApps->Add( pFwApp ); 
 
LCleanup: 
    if( bstrFriendlyAppName ) SysFreeString( bstrFriendlyAppName ); 
    if( bstrGameExeFullPath ) SysFreeString( bstrGameExeFullPath ); 
    if( pFwApp ) pFwApp->Release(); 
    if( pFwApps ) pFwApps->Release(); 
    if( pFwProfile ) pFwProfile->Release(); 
    if( bCleanupCOM ) CoUninitialize(); 
 
    return hr; 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Removes application from exception list 
//-------------------------------------------------------------------------------------- 
HRESULT RemoveApplicationFromExceptionListW( WCHAR* strGameExeFullPath ) 
{ 
    HRESULT hr = E_FAIL; 
    bool bCleanupCOM = false; 
    BSTR bstrGameExeFullPath = NULL; 
    INetFwAuthorizedApplications* pFwApps = NULL; 
    INetFwProfile* pFwProfile = NULL; 
 
#ifdef SHOW_DEBUG_MSGBOXES 
        WCHAR sz[1024]; 
        StringCchPrintf( sz, 1024, L"strGameExeFullPath='%s'", strGameExeFullPath ); 
        MessageBox( NULL, sz, L"RemoveApplicationFromExceptionListW", MB_OK ); 
#endif 
 
    if( strGameExeFullPath == NULL ) 
    { 
		PC_ASSERT(false, "strGameExeFullPath == NULL");
        return E_INVALIDARG; 
    } 
 
    bstrGameExeFullPath = SysAllocString( strGameExeFullPath ); 
    if( bstrGameExeFullPath == NULL ) 
    { 
        hr = E_OUTOFMEMORY; 
        goto LCleanup; 
    } 
 
    hr = CoInitialize( 0 ); 
    bCleanupCOM = SUCCEEDED( hr ); 
 
    pFwProfile = GetFirewallProfile(); 
    if( pFwProfile == NULL ) 
    { 
        hr = E_FAIL; 
        goto LCleanup; 
    } 
 
    // Retrieve the authorized application collection. 
    hr = pFwProfile->get_AuthorizedApplications( &pFwApps ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Remove the application from the collection. 
    hr = pFwApps->Remove( bstrGameExeFullPath ); 
 
LCleanup: 
    if( pFwProfile ) pFwProfile->Release(); 
    if( bCleanupCOM ) CoUninitialize(); 
    if( bstrGameExeFullPath ) SysFreeString( bstrGameExeFullPath ); 
 
    return hr; 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Adds application from exception list 
//-------------------------------------------------------------------------------------- 
HRESULT AddApplicationToExceptionListA( CHAR* strGameExeFullPath, CHAR* strFriendlyAppName ) 
{ 
    WCHAR wstrPath[MAX_PATH] = {0}; 
    WCHAR wstrName[MAX_PATH] = {0}; 
 
    MultiByteToWideChar( CP_ACP, 0, strGameExeFullPath, MAX_PATH, wstrPath, MAX_PATH ); 
    MultiByteToWideChar( CP_ACP, 0, strFriendlyAppName, MAX_PATH, wstrName, MAX_PATH ); 
 
    return AddApplicationToExceptionListW( wstrPath, wstrName ); 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Removes application from exception list 
//-------------------------------------------------------------------------------------- 
HRESULT RemoveApplicationFromExceptionListA( CHAR* strGameExeFullPath ) 
{ 
    WCHAR wstrPath[MAX_PATH] = {0}; 
 
    MultiByteToWideChar( CP_ACP, 0, strGameExeFullPath, MAX_PATH, wstrPath, MAX_PATH ); 
 
    return RemoveApplicationFromExceptionListW( wstrPath ); 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Returns false if the game is not allowed through the firewall 
//-------------------------------------------------------------------------------------- 
BOOL WINAPI CanLaunchMultiplayerGameW( WCHAR* strGameExeFullPath ) 
{ 
    bool bCanLaunch = false; 
    HRESULT hr = E_FAIL; 
    bool bCleanupCOM = false; 
    BSTR bstrGameExeFullPath = NULL; 
    VARIANT_BOOL vbFwEnabled; 
    VARIANT_BOOL vbNotAllowed = VARIANT_FALSE; 
    INetFwAuthorizedApplication* pFwApp = NULL; 
    INetFwAuthorizedApplications* pFwApps = NULL; 
    INetFwProfile* pFwProfile = NULL; 
 
    if( strGameExeFullPath == NULL ) 
    { 
		PC_ASSERT(false, "strGameExeFullPath == NULL");
        return false; 
    } 
 
    bstrGameExeFullPath = SysAllocString( strGameExeFullPath ); 
    if( bstrGameExeFullPath == NULL ) 
    { 
        hr = E_OUTOFMEMORY; 
        goto LCleanup; 
    } 
 
    hr = CoInitialize( 0 ); 
    bCleanupCOM = SUCCEEDED( hr ); 
 
    pFwProfile = GetFirewallProfile(); 
    if( pFwProfile == NULL ) 
    { 
        hr = E_FAIL; 
        goto LCleanup; 
    } 
 
    hr = pFwProfile->get_ExceptionsNotAllowed( &vbNotAllowed ); 
    if( SUCCEEDED( hr ) && vbNotAllowed != VARIANT_FALSE ) 
        goto LCleanup; 
 
    // Retrieve the collection of authorized applications. 
    hr = pFwProfile->get_AuthorizedApplications( &pFwApps ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Attempt to retrieve the authorized application. 
    hr = pFwApps->Item( bstrGameExeFullPath, &pFwApp ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Find out if the authorized application is enabled. 
    hr = pFwApp->get_Enabled( &vbFwEnabled ); 
    if( FAILED( hr ) ) 
        goto LCleanup; 
 
    // Check if authorized application is enabled. 
    if( vbFwEnabled != VARIANT_FALSE ) 
        bCanLaunch = true; 
 
LCleanup: 
    if( pFwApp ) pFwApp->Release(); 
    if( pFwApps ) pFwApps->Release(); 
    if( bCleanupCOM ) CoUninitialize(); 
    if( bstrGameExeFullPath ) SysFreeString( bstrGameExeFullPath ); 
 
    return bCanLaunch; 
} 
 
 
//-------------------------------------------------------------------------------------- 
// Returns false if the game is not allowed through the firewall 
//-------------------------------------------------------------------------------------- 
BOOL WINAPI CanLaunchMultiplayerGameA( CHAR* strGameExeFullPath ) 
{ 
    WCHAR wstrPath[MAX_PATH] = {0}; 
 
    MultiByteToWideChar( CP_ACP, 0, strGameExeFullPath, MAX_PATH, wstrPath, MAX_PATH ); 
 
    return CanLaunchMultiplayerGameW( wstrPath ); 
} 
 
 

#endif	// !defined(_WIN32)


//////////////////////////////////////////////////////////////////////////
PCLIB_NAMESPACE_END
//////////////////////////////////////////////////////////////////////////