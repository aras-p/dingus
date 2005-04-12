// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include <windowsx.h>
#include "D3DSaver.h"
#include "DXUtil.h"
//#include <commctrl.h>

#include <regstr.h>
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#include <mmsystem.h>

using namespace dingus::ss;


// Resource IDs.  D3DSaver assumes that you will create resources with
// these IDs that it can use.  The easiest way to do this is to copy
// the resources from the rc file of an existing D3DSaver-based program.
#define IDI_MAIN_ICON					101
#define IDD_SINGLEMONITORSETTINGS		200
//#define IDD_MULTIMONITORSETTINGS		201

//#define IDC_MONITORSTAB 				2000
//#define IDC_TABNAMEFMT					2001
#define IDC_ADAPTERNAME 				2002
#define IDC_RENDERING					2003
#define IDC_MOREINFO					2004
#define IDC_DISABLEHW					2005
#define IDC_SCREENUSAGEBOX				2006
//#define IDC_RENDER						2007
//#define IDC_LEAVEBLACK					2008
#define IDC_DISPLAYMODEBOX				2009
#define IDC_MODESSTATIC 				2010
#define IDC_MODESCOMBO					2011
#define IDC_AUTOMATIC					2012
#define IDC_DISPLAYMODENOTE 			2013
#define IDC_GENERALBOX					2014
//#define IDC_SAME						2015
#define IDC_MODEFMT 					2016

#define IDS_ERR_GENERIC 				2100
#define IDS_ERR_NODIRECT3D				2101
#define IDS_ERR_NOWINDOWEDHAL			2102
#define IDS_ERR_CREATEDEVICEFAILED		2103
#define IDS_ERR_NOCOMPATIBLEDEVICES 	2104
#define IDS_ERR_NOHARDWAREDEVICE		2105
#define IDS_ERR_HALNOTCOMPATIBLE		2106
#define IDS_ERR_NOHALTHISMODE			2107
#define IDS_ERR_MEDIANOTFOUND			2108
#define IDS_ERR_RESIZEFAILED			2109
#define IDS_ERR_OUTOFMEMORY 			2110
#define IDS_ERR_OUTOFVIDEOMEMORY		2111
#define IDS_ERR_NOPREVIEW				2112

#define IDS_INFO_GOODHAL				2200
#define IDS_INFO_BADHAL_GOODSW			2201
#define IDS_INFO_BADHAL_BADSW			2202
#define IDS_INFO_BADHAL_NOSW			2203
#define IDS_INFO_NOHAL_GOODSW			2204
#define IDS_INFO_NOHAL_BADSW			2205
#define IDS_INFO_NOHAL_NOSW 			2206
#define IDS_INFO_DISABLEDHAL_GOODSW 	2207
#define IDS_INFO_DISABLEDHAL_BADSW		2208
#define IDS_INFO_DISABLEDHAL_NOSW		2209
#define IDS_RENDERING_HAL				2210
#define IDS_RENDERING_SW				2211
#define IDS_RENDERING_NONE				2212


// Use the following structure rather than DISPLAY_DEVICE, since some old 
// versions of DISPLAY_DEVICE are missing the last two fields and this can
// cause problems with EnumDisplayDevices on Windows 2000.
struct DISPLAY_DEVICE_FULL {
	DWORD  cb;
	TCHAR  DeviceName[32];
	TCHAR  DeviceString[128];
	DWORD  StateFlags;
	TCHAR  DeviceID[128];
	TCHAR  DeviceKey[128];
};


static CD3DScreensaver* gD3DScreensaver = NULL;



// --------------------------------------------------------------------------

/*
CD3DScreensaver::CD3DScreensaver()
{
	gD3DScreensaver = this;
	
	mCheckingPassword = false;
	mIsWin9x = false;
	mMouseMoveCount = 0;
	mHwndParent = NULL;
	mPasswordDLL = NULL;
	mHwnd = NULL;
	mVerifyPasswordProc = NULL;
	
	//mAllScreensSame = false;
	mD3D = NULL;
	mD3DDevice = NULL;
	mWindowed = false;
	mInStartingPause = false;
	
	mErrorMode = false;
	mErrorHR = S_OK;
	mErrorText[0] = TEXT('\0');
	
	mFPS			= 0.0f;
	mDeviceStats[0] = TEXT('\0');
	mFrameStats[0]	= TEXT('\0');
	
	// Note: clients should load a resource into mWindowTitle to localize this string
	lstrcpy( mWindowTitle, TEXT("Screen Saver") );
	mAllowRef		= false;
	mUseDepthBuffer	= false;
	mMultithreaded	= false;
	mRegistryPath[0] = TEXT('\0');
	mMinDepthBits	= 16;
	mMinStencilBits	= 0;
	mSwapFxFullscreen	= D3DSWAPEFFECT_DISCARD;
	mSwapFxWindowed		= D3DSWAPEFFECT_COPY;
	
	//SetRectEmpty( &mRenderAreaTotal );
	//SetRectEmpty( &mRenderAreaCurr );
	
	//ZeroMemory( mMonitors, sizeof(mMonitors) );
	//mMonitorCount = 0;
	ZeroMemory( mAdapters, sizeof(mAdapters) );
	mAdapterCount = 0;

	mCurrAdapterIdx = 0;
	//mCurrMonitorIdx = 0;
	mCurrDevType = (D3DDEVTYPE)0;
	mCurrBehavior = 0;
	ZeroMemory( &mCurrPresentParams, sizeof(mCurrPresentParams) );
	mDevObjsInited = false;
	mDevObjsRestored = false;
	
	mTime = 0.0f;
	mElapsedTime = 0.0f;
}


// --------------------------------------------------------------------------

/// Have the client program call this function before calling run().

HRESULT CD3DScreensaver::create( HINSTANCE hInstance )
{
	HRESULT hr;
	
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
	
	mInstance = hInstance;
	
	// Parse the command line and do the appropriate thing
	TCHAR* cmdLine = GetCommandLine();
	mSaverMode = parseCmdLine( cmdLine );
	
	//enumMonitors();
	
	// create the screen saver window(s)
	if( mSaverMode == SM_PREVIEW || mSaverMode == SM_TEST || mSaverMode == SM_FULL ) {
		if( FAILED( hr = createSaverWindow() ) ) {
			mErrorMode = true;
			mErrorHR = hr;
		}
	}
	
	if( mSaverMode == SM_PREVIEW ) {
		// In preview mode, "pause" (enter a limited message loop) briefly 
		// before proceeding, so the display control panel knows to update itself.
		mInStartingPause = true;
		
		// Post a message to mark the end of the initial group of window messages
		PostMessage( mHwnd, WM_USER, 0, 0 );
		
		MSG msg;
		while( mInStartingPause ) {
			// If GetMessage returns false, it's quitting time.
			if( !GetMessage( &msg, mHwnd, 0, 0 ) ) {
				// Post the quit message to handle it later
				PostQuitMessage(0);
				break;
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	
	// create Direct3D object
	if( (mD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == NULL ) {
		mErrorMode = true;
		mErrorHR = D3DAPPERR_NODIRECT3D;
		return S_OK;
	}
	
	// Give the app the opportunity to register a pluggable SW D3D Device.
	if( FAILED( hr = registerSWDevice() ) ) {
		mErrorMode = true;
		mErrorHR = hr;
		return S_OK;
	}
	
	// Build a list of Direct3D adapters, modes and devices. The
	// confirmDevice() callback is used to confirm that only devices that
	// meet the app's requirements are considered.
	if( FAILED( hr = buildDeviceList() ) ) {
		mErrorMode = true;
		mErrorHR = hr;
		return S_OK;
	}
	
	// Make sure that at least one valid usable D3D device was found
	bool bCompatibleDeviceFound = false;
	for( DWORD adapterIdx = 0; adapterIdx < mAdapterCount; adapterIdx++ ) {
		if( mAdapters[adapterIdx]->hasNeededHAL || mAdapters[adapterIdx]->hasNeededSW ) {
			bCompatibleDeviceFound = true;
			break;
		}
	}
	if( !bCompatibleDeviceFound ) {
		mErrorMode = true;
		mErrorHR = D3DAPPERR_NOCOMPATIBLEDEVICES;
		return S_OK;
	}
	
	// Read any settings we need
	readSettings();
	
	return S_OK;
}
*/


// --------------------------------------------------------------------------
//	Determine HMONITOR, desktop rect, and other info for each monitor.
//	Note that EnumDisplayDevices enumerates monitors in the order
//	indicated on the Settings page of the Display control panel, which
//	is the order we want to list monitors in, as opposed to the order
//	used by D3D's GetAdapterInfo.

/*
void CD3DScreensaver::enumMonitors( void )
{
	DWORD devIdx = 0;
	DISPLAY_DEVICE_FULL dispdev;
	DISPLAY_DEVICE_FULL dispdev2;
	DEVMODE devmode;
	dispdev.cb = sizeof(dispdev);
	dispdev2.cb = sizeof(dispdev2);
	devmode.dmSize = sizeof(devmode);
	devmode.dmDriverExtra = 0;
	while( EnumDisplayDevices( NULL, devIdx, (DISPLAY_DEVICE*)&dispdev, 0 ) ) {
		
		// Ignore NetMeeting's mirrored displays
		if( (dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0 ) {
			// To get monitor info for a display device, call EnumDisplayDevices
			// a second time, passing dispdev.DeviceName (from the first call) as
			// the first parameter.
			EnumDisplayDevices( dispdev.DeviceName, 0, (DISPLAY_DEVICE*)&dispdev2, 0 );
			
			SMonitorInfo& moninfo = mMonitors[mMonitorCount];
			ZeroMemory( &moninfo, sizeof(SMonitorInfo) );
			lstrcpy( moninfo.deviceName, dispdev.DeviceString );
			lstrcpy( moninfo.monitorName, dispdev2.DeviceString );
			moninfo.adapterIdx = NO_ADAPTER;
			
			if( dispdev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP ) {
				EnumDisplaySettings( dispdev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode );
				if( dispdev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE ) {
					// For some reason devmode.dmPosition is not always (0, 0)
					// for the primary display, so force it.
					moninfo.screenRect.left = 0;
					moninfo.screenRect.top = 0;
				} else {
					moninfo.screenRect.left = devmode.dmPosition.x;
					moninfo.screenRect.top = devmode.dmPosition.y;
				}
				moninfo.screenRect.right = moninfo.screenRect.left + devmode.dmPelsWidth;
				moninfo.screenRect.bottom = moninfo.screenRect.top + devmode.dmPelsHeight;
				moninfo.hMonitor = MonitorFromRect( &moninfo.screenRect, MONITOR_DEFAULTTONULL );
			}
			mMonitorCount++;
			if( mMonitorCount == MAX_DISPLAYS )
				break;
		}
		devIdx++;
	}
}
*/


// --------------------------------------------------------------------------
//  Starts main execution of the screen saver.

/*
INT CD3DScreensaver::run()
{
	HRESULT hr;
	
	switch ( mSaverMode ) {
	case SM_CONFIG:
		if( mErrorMode ) {
			displayErrorMsg( mErrorHR, 0 );
		} else {
			doConfig();
		}
		break;
	case SM_PREVIEW:
	case SM_TEST:
	case SM_FULL:
		if( FAILED( hr = doSaver() ) )
			displayErrorMsg( hr, 0 );
		break;
	case SM_PASSCHANGE:
		changePassword();
		break;
	}
	
	for( DWORD adapterIdx = 0; adapterIdx < mAdapterCount; adapterIdx++ )
		safeDelete( mAdapters[adapterIdx] );
	safeRelease( mD3D );
	return 0;
}
*/


// --------------------------------------------------------------------------
//  Interpret command-line parameters passed to this app.

/*
eSaverMode CD3DScreensaver::parseCmdLine( TCHAR* cmdLine )
{
	mHwndParent = NULL;
	
	// Skip the first part of the command line, which is the full path 
	// to the exe.	If it contains spaces, it will be contained in quotes.
	if( *cmdLine == TEXT('\"') ) {
		cmdLine++;
		while( *cmdLine != TEXT('\0') && *cmdLine != TEXT('\"') )
			cmdLine++;
		if( *cmdLine == TEXT('\"') )
			cmdLine++;
	} else {
		while( *cmdLine != TEXT('\0') && *cmdLine != TEXT(' ') )
			cmdLine++;
		if( *cmdLine == TEXT(' ') )
			cmdLine++;
	}
	
	// Skip along to the first option delimiter "/" or "-"
	while( *cmdLine != TEXT('\0') && *cmdLine != TEXT('/') && *cmdLine != TEXT('-') )
		cmdLine++;
	
	// If there wasn't one, then must be config mode
	if( *cmdLine == TEXT('\0') )
		return SM_CONFIG;
	
	// Otherwise see what the option was
	switch( *(++cmdLine) ) {
	case 'c':
	case 'C':
		cmdLine++;
		while ( *cmdLine && !isdigit(*cmdLine) )
			cmdLine++;
		if( isdigit(*cmdLine) ) {
#ifdef _WIN64
			CHAR strCommandLine[2048];
			DXUtil_ConvertGenericStringToAnsiCb( strCommandLine, cmdLine, sizeof(strCommandLine) );
			mHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
#endif
		} else {
			mHwndParent = NULL;
		}
		return SM_CONFIG;
		
	case 't':
	case 'T':
		return SM_TEST;
		
	case 'p':
	case 'P':
		// Preview-mode, so option is followed by the parent HWND in decimal
		cmdLine++;
		while( *cmdLine && !isdigit(*cmdLine) )
			cmdLine++;
		if( isdigit(*cmdLine) ) {
#ifdef _WIN64
			CHAR strCommandLine[2048];
			DXUtil_ConvertGenericStringToAnsiCb(strCommandLine, cmdLine, sizeof(strCommandLine));
			mHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
#endif
		}
		return SM_PREVIEW;
		
	case 'a':
	case 'A':
		// Password change mode, so option is followed by parent HWND in decimal
		cmdLine++;
		while( *cmdLine && !isdigit(*cmdLine) )
			cmdLine++;
		if ( isdigit(*cmdLine) ) {
#ifdef _WIN64
			CHAR strCommandLine[2048];
			DXUtil_ConvertGenericStringToAnsiCb(strCommandLine, cmdLine, sizeof(strCommandLine));
			mHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
#endif
		}
		return SM_PASSCHANGE;
		
	default:
		// All other options => run the screensaver (typically this is "/s")
		return SM_FULL;
	}
}
*/

// --------------------------------------------------------------------------
//  Register and create the appropriate window(s)

/*
HRESULT CD3DScreensaver::createSaverWindow()
{
	// Uncomment this code to allow stepping thru code in the preview case
	if( mSaverMode == SM_PREVIEW )
	{
	WNDCLASS cls;
	cls.hCursor 	   = NULL; 
	cls.hIcon		   = NULL; 
	cls.lpszMenuName   = NULL;
	cls.lpszClassName  = TEXT("Parent"); 
	cls.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
	cls.hInstance	   = mInstance; 
	cls.style		   = CS_VREDRAW|CS_HREDRAW|CS_SAVEBITS|CS_DBLCLKS;
	cls.lpfnWndProc    = DefWindowProc;
	cls.cbWndExtra	   = 0; 
	cls.cbClsExtra	   = 0; 
	RegisterClass( &cls );

	// create the window
	RECT rect;
	HWND hwnd;
	rect.left = rect.top = 40;
	rect.right = rect.left+200;
	rect.bottom = rect.top+200;
	AdjustWindowRect( &rect, WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_POPUP, false );
	hwnd = CreateWindow( TEXT("Parent"), TEXT("FakeShell"),
	WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_POPUP, rect.left, rect.top,
	rect.right-rect.left, rect.bottom-rect.top, NULL,
	NULL, mInstance, NULL );
	mHwndParent = hwnd;
	}
	
	// Register an appropriate window class
	WNDCLASS	cls;
	cls.hCursor 	   = LoadCursor( NULL, IDC_ARROW );
	cls.hIcon		   = LoadIcon( mInstance, MAKEINTRESOURCE(IDI_MAIN_ICON) ); 
	cls.lpszMenuName   = NULL;
	cls.lpszClassName  = TEXT("D3DSaverWndClass");
	cls.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
	cls.hInstance	   = mInstance; 
	cls.style		   = CS_VREDRAW|CS_HREDRAW;
	cls.lpfnWndProc    = saverProcStub;
	cls.cbWndExtra	   = 0; 
	cls.cbClsExtra	   = 0; 
	RegisterClass( &cls );
	
	// create the window
	RECT rc;
	DWORD dwStyle;
	switch( mSaverMode ) {
	case SM_PREVIEW:
		GetClientRect( mHwndParent, &rc );
		dwStyle = WS_VISIBLE | WS_CHILD;
		AdjustWindowRect( &rc, dwStyle, false );
		mHwnd = CreateWindow( TEXT("D3DSaverWndClass"), mWindowTitle, dwStyle, 
			rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
			mHwndParent, NULL, mInstance, this );
		//mMonitors[0].hwnd = mHwnd;
		//GetClientRect( mHwnd, &mRenderAreaTotal );
		//GetClientRect( mHwnd, &mRenderAreaCurr );
		break;
		
	case SM_TEST:
		rc.left = rc.top = 50;
		rc.right = rc.left+600;
		rc.bottom = rc.top+400;
		dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		AdjustWindowRect( &rc, dwStyle, false );
		mHwnd = CreateWindow( TEXT("D3DSaverWndClass"), mWindowTitle, dwStyle, 
			rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
			NULL, NULL, mInstance, this );
		//mMonitors[0].hwnd = mHwnd;
		//GetClientRect( mHwnd, &mRenderAreaTotal );
		//GetClientRect( mHwnd, &mRenderAreaCurr );
		break;
		
	case SM_FULL:
		// create (dummy) fullscreen window.
		rc.left = rc.top = 50;
		rc.right = rc.left+600;
		rc.bottom = rc.top+400;
		dwStyle = WS_VISIBLE | WS_POPUP;
		AdjustWindowRect( &rc, dwStyle, false );
		mHwnd = CreateWindowEx( WS_EX_TOPMOST, TEXT("D3DSaverWndClass"), 
			mWindowTitle, dwStyle, rc.left, rc.top, rc.right - rc.left, 
			rc.bottom - rc.top, NULL, NULL, mInstance, this );
	}
	if ( mHwnd == NULL )
		return E_FAIL;
	
	return S_OK;
}
*/


// --------------------------------------------------------------------------
//  Run the screensaver graphics - may be preview, test or full-on mode
/*
HRESULT CD3DScreensaver::doSaver()
{
	HRESULT hr;
	
	// Figure out if we're on Win9x
	OSVERSIONINFO osvi; 
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx( &osvi );
	mIsWin9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	
	// If we're in full on mode, and on 9x, then need to load the password DLL
	if( mSaverMode == SM_FULL && mIsWin9x ) {
		// Only do this if the password is set - check registry:
		HKEY hKey; 
		if( RegCreateKeyEx( HKEY_CURRENT_USER, REGSTR_PATH_SCREENSAVE, 0, NULL, 0, KEY_READ, NULL, &hKey, NULL ) == ERROR_SUCCESS ) { 
			DWORD dwVal;
			DWORD dwSize = sizeof(dwVal); 
			if ( (RegQueryValueEx( hKey, REGSTR_VALUE_USESCRPASSWORD, NULL, NULL,
				(BYTE*)&dwVal, &dwSize ) == ERROR_SUCCESS) && dwVal ) 
			{ 
				mPasswordDLL = LoadLibrary( TEXT("PASSWORD.CPL") );
				if( mPasswordDLL )
					mVerifyPasswordProc = (VERIFYPWDPROC)GetProcAddress( mPasswordDLL, "VerifyScreenSavePwd" );
				RegCloseKey( hKey );
			}
		}
	}
	
	// Initialize the application timer
	dingus::timer( TIMER_START );
	
	if( !mErrorMode ) {
		// Initialize the app's custom scene stuff
		if( FAILED( hr = initialize() ) )
			return displayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		// Do graphical init stuff
		if ( FAILED(hr = initialize3DEnvironment()) )
			return hr;
	}
	
	// Flag as screensaver running if in full on mode
	if( mSaverMode == SM_FULL ) {
		bool bUnused;
		SystemParametersInfo( SPI_SCREENSAVERRUNNING, true, &bUnused, 0 );
	}
	
	// Message pump
	BOOL gotMsg;
	MSG msg;
	msg.message = WM_NULL;
	while ( msg.message != WM_QUIT ) {
		gotMsg = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
		if( gotMsg ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		} else {
			Sleep( 2 );
			if( mErrorMode ) {
				updateErrorBox();
			} else {
				render3DEnvironment();
			}
		}
	}
	
	return S_OK;
}
*/

// --------------------------------------------------------------------------
/*
void CD3DScreensaver::shutdownSaver()
{
	// Unflag screensaver running if in full on mode
	if( mSaverMode == SM_FULL ) {
		bool bUnused;
		SystemParametersInfo( SPI_SCREENSAVERRUNNING, false, &bUnused, 0 );
	}
	
	// Kill graphical stuff
	cleanup3DEnvironment();
	
	// Let client app clean up its resources
	shutdown();
	
	// Unload the password DLL (if we loaded it)
	if( mPasswordDLL != NULL ) {
		FreeLibrary( mPasswordDLL );
		mPasswordDLL = NULL;
	}
	
	// Post message to drop out of message loop
	PostQuitMessage( 0 );
}
*/


// --------------------------------------------------------------------------

/*
LRESULT CALLBACK CD3DScreensaver::saverProcStub( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return gD3DScreensaver->saverProc( hwnd, msg, wParam, lParam );
}


// Handle window messages for main screensaver windows (one per screen).

LRESULT CD3DScreensaver::saverProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_USER:
		// All initialization messages have gone through.  Allow
		// 500ms of idle time, then proceed with initialization.
		SetTimer( hwnd, 1, 500, NULL );
		break;
		
	case WM_TIMER:
		// Initial idle time is done, proceed with initialization.
		mInStartingPause = false;
		KillTimer( hwnd, 1 );
		break;
		
	case WM_DESTROY:
		shutdownSaver();
		break;
		
	case WM_SETCURSOR:
		if( mSaverMode == SM_FULL && !mCheckingPassword ) {
			// Hide cursor
			SetCursor( NULL );
			return true;
		}
		break;
		
	case WM_PAINT:
		{
			// Show mErroror message, if there is one
			PAINTSTRUCT ps;
			BeginPaint( hwnd, &ps );
			// In preview mode, just fill 
			// the preview window with black. 
			if( !mErrorMode && mSaverMode == SM_PREVIEW ) {
				RECT rc;
				GetClientRect(hwnd,&rc);
				FillRect(ps.hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH) );
			} else {
				doPaint( hwnd, ps.hdc );
			}
			EndPaint( hwnd, &ps );
			return 0;
		}
		
	case WM_ERASEBKGND:
		// Erase background if checking password
		if( !mCheckingPassword )
			return true; // don't erase this window
		break;
		
	case WM_MOUSEMOVE:
		if( mSaverMode != SM_TEST ) {
			static INT xPrev = -1;
			static INT yPrev = -1;
			INT xCur = GET_X_LPARAM(lParam);
			INT yCur = GET_Y_LPARAM(lParam);
			if( xCur != xPrev || yCur != yPrev )
			{
				xPrev = xCur;
				yPrev = yCur;
				mMouseMoveCount++;
				if ( mMouseMoveCount > 5 )
					interruptSaver();
			}
		}
		break;
		
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if( mSaverMode != SM_TEST )
			interruptSaver();
		break;
		
	case WM_ACTIVATEAPP:
		if( wParam == false && mSaverMode != SM_TEST )
			interruptSaver();
		break;
		
	case WM_POWERBROADCAST:
		if( wParam == PBT_APMSUSPEND && mVerifyPasswordProc == NULL )
			interruptSaver();
		break;
		
	case WM_SYSCOMMAND: 
		if ( mSaverMode == SM_FULL ) {
			switch ( wParam ) {
			case SC_NEXTWINDOW:
			case SC_PREVWINDOW:
			case SC_SCREENSAVE:
			case SC_CLOSE:
				return false;
			};
		}
		break;
	}
	
	return DefWindowProc( hwnd, msg, wParam, lParam );
}

*/


// --------------------------------------------------------------------------

/*
//  A message was received (mouse move, keydown, etc.) that may mean
//	the screen saver should show the password dialog and/or shut down.
void CD3DScreensaver::interruptSaver()
{
	HRESULT hr;
	bool passwordOK = false;
	
	if( mSaverMode == SM_TEST || mSaverMode == SM_FULL && !mCheckingPassword ) {
		if( mIsWin9x && mSaverMode == SM_FULL ) {
			// If no VerifyPassword function, then no password is set 
			// or we're not on 9x. 
			if( mVerifyPasswordProc != NULL ) {
				// Shut down D3D device so we can show a Windows dialog
				//switchToRenderUnit( iru );
				if( mDevObjsRestored ) {
					passivateDeviceObjects();
					mDevObjsRestored = false;
				}
				if( mDevObjsInited ) {
					deleteDeviceObjects();
					mDevObjsInited = false;
				}
				safeRelease(mD3DDevice);
				
				// Make sure window covers the whole screen,
				// even after deleting D3D device (which may have caused
				// mode changes)
				ShowWindow( mHwnd, SW_RESTORE );
				ShowWindow( mHwnd, SW_MAXIMIZE );
				
				mCheckingPassword = true;
				
				passwordOK = mVerifyPasswordProc( mHwnd );
				
				mCheckingPassword = false;
				
				if( passwordOK ) {
					// D3D devices are all torn down, so it's safe
					// to discard all render units now (so we don't
					// try to clean them up again later).
				} else {
					// Back to screen saving...
					SetCursor( NULL );
					mMouseMoveCount = 0;
					
					// Recreate D3D device
					hr = mD3D->CreateDevice(
						mCurrAdapterIdx, 
						mCurrDevType, mHwnd, 
						mCurrBehavior, &mCurrPresentParams, 
						&mD3DDevice );
					if( FAILED( hr ) ) {
						mErrorMode = true;
						mErrorHR = D3DAPPERR_CREATEDEVICEFAILED;
					} else {
						//switchToRenderUnit(iru);
						if( FAILED(hr = createDeviceObjects() ) ) {
							mErrorMode = true;
							mErrorHR = D3DAPPERR_INITDEVICEOBJECTSFAILED;
						} else  {
							mDevObjsInited = true;
							if( FAILED(hr = activateDeviceObjects() ) ) {
								mErrorMode = true;
								mErrorHR = D3DAPPERR_INITDEVICEOBJECTSFAILED;
							} else {
								mDevObjsRestored = true;
							}
						}
					}
					
					return;
				}
			}
		}
		shutdownSaver();
	}
}
*/

// --------------------------------------------------------------------------

// Set up D3D device(s)
/*
HRESULT CD3DScreensaver::initialize3DEnvironment()
{
	HRESULT hr;
	//DWORD adapterIdx;
	//UINT monIdx;
	SD3DAdapterInfo* adapterInfo;
	//SMonitorInfo* monInfo;
	//MONITORINFO monitorInfo;
	
	if( mSaverMode == SM_FULL ) {
		// Fullscreen mode.  create a SRenderUnit for each monitor (unless 
		// the user wants it black)
		mWindowed = false;
		
		// Get the best adapter
		getBestAdapter( &mCurrAdapterIdx );
		if( mCurrAdapterIdx == NO_ADAPTER ) {
			mErrorMode = true;
			mErrorHR = D3DAPPERR_NOCOMPATIBLEDEVICES;
		} else {
			adapterInfo = mAdapters[mCurrAdapterIdx];
			if( adapterInfo->disableHAL && !adapterInfo->hasNeededSW && !mAllowRef ) {
			} else if( adapterInfo->deviceCount > 0 ) {
				mCurrDevType = (D3DDEVTYPE)0;
				mCurrBehavior = 0;
				ZeroMemory( &mCurrPresentParams, sizeof(mCurrPresentParams) );
				mDevObjsInited = false;
				mDevObjsRestored = false;

				if( FAILED( hr = createFullscreen() ) ) {
					mErrorMode = true;
					mErrorHR = D3DAPPERR_CREATEDEVICEFAILED;
				}
			}
		}
		
	} else {
		
		// Windowed mode, for test mode or preview window
		mWindowed = true;
		
		//GetClientRect( mHwnd, &mRenderAreaTotal );
		//GetClientRect( mHwnd, &mRenderAreaCurr );
		
		getBestAdapter( &mCurrAdapterIdx );
		if( mCurrAdapterIdx == NO_ADAPTER ) {
			mErrorMode = true;
			mErrorHR = D3DAPPERR_CREATEDEVICEFAILED;
		}

		if( !mErrorMode ) {
			mCurrDevType = (D3DDEVTYPE)0;
			mCurrBehavior = 0;
			ZeroMemory( &mCurrPresentParams, sizeof(mCurrPresentParams) );
			mDevObjsInited = false;
			mDevObjsRestored = false;

			if( FAILED( hr = createWindowed() ) ) {
				mErrorMode = true;
				if( mSaverMode == SM_PREVIEW )
					mErrorHR = D3DAPPERR_NOPREVIEW;
				else
					mErrorHR = D3DAPPERR_CREATEDEVICEFAILED;
			}
		}
	}
	
	// Once all mode changes are done, (re-)determine coordinates of all 
	// screens, and make sure windows still cover each screen
	//*
	for( monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo( monInfo->hMonitor, &monitorInfo );
		monInfo->screenRect = monitorInfo.rcMonitor;
		if( !mWindowed ) {
			SetWindowPos( monInfo->hwnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, 
				monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, 
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOACTIVATE );
		}
	}
	//*
	
	//*
	// For fullscreen, determine bounds of the virtual screen containing all 
	// screens that are rendering.	Don't just use SM_XVIRTUALSCREEN, because 
	// we don't want to count screens that are just black
	if( !mWindowed ) {
		for( iru = 0; iru < mRenderUnitCount; iru++ ) {
			ru = &mRenderUnits[iru];
			monInfo = &mMonitors[ru->monIdx];
			UnionRect( &mRenderAreaTotal, &mRenderAreaTotal, &monInfo->screenRect );
		}
	}
	//*
	
	if( !mErrorMode ) {
		// Initialize D3D device
		//switchToRenderUnit( iru );
		if ( FAILED(hr = createDeviceObjects() ) ) {
			mErrorMode = true;
			mErrorHR = D3DAPPERR_INITDEVICEOBJECTSFAILED;
		} else  {
			mDevObjsInited = true;
			if ( FAILED(hr = activateDeviceObjects() ) ) {
				mErrorMode = true;
				mErrorHR = D3DAPPERR_INITDEVICEOBJECTSFAILED;
			} else {
				mDevObjsRestored = true;
			}
		}
		updateDeviceStats(); 
	}
	
	// Make sure all those display changes don't count as user mouse moves
	mMouseMoveCount = 0;
	
	return S_OK;
}
*/

// --------------------------------------------------------------------------

// To decide which adapter to use, loop through monitors until you find
// one whose adapter has a compatible HAL.  If none, use the first 
// monitor that has an compatible SW device.
/*
bool CD3DScreensaver::getBestAdapter( UINT* adapter )
{
	UINT bestAdapterIdx = NO_ADAPTER;
	UINT adapterIdx;
	//DWORD monIdx;
	//SMonitorInfo* monInfo;
	SD3DAdapterInfo* adapterInfo;
	
	for( monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		adapterIdx = monInfo->adapterIdx;
		if( adapterIdx == NO_ADAPTER )
			continue; 
		adapterInfo = mAdapters[adapterIdx];
		if( adapterInfo->hasNeededHAL ) {
			bestAdapterIdx = adapterIdx;
			break;
		}
		if( adapterInfo->hasNeededSW )
		{
			bestAdapterIdx = adapterIdx;
			// but keep looking...
		}
	}
	*adapter = bestAdapterIdx;
	
	return (bestAdapterIdx != NO_ADAPTER);
}
*/

// --------------------------------------------------------------------------

/*
HRESULT CD3DScreensaver::createFullscreenRenderUnit( SRenderUnit* ru )
{
	HRESULT hr;
	UINT adapterIdx = mCurrAdapterIdx;
	SD3DAdapterInfo* adapterInfo = mAdapters[adapterIdx];
	//DWORD monIdx = adapterInfo->monIdx;
	SD3DDeviceInfo* devInfo;
	SD3DModeInfo* modeInfo;
	DWORD currDevice;
	D3DDEVTYPE curType;
	
	if( adapterIdx >= mAdapterCount )
		return E_FAIL;
	
	if( adapterInfo->deviceCount == 0 )
		return E_FAIL;
	
	// Find the best device for the adapter.  Use HAL
	// if it's there, otherwise SW, otherwise REF.
	currDevice = 0xffff;
	curType = D3DDEVTYPE_FORCE_DWORD;
	for( DWORD devIdx = 0; devIdx < adapterInfo->deviceCount; devIdx++ ) {
		devInfo = &adapterInfo->devices[devIdx];
		if( devInfo->devType == D3DDEVTYPE_HAL && !adapterInfo->disableHAL ) {
			currDevice = devIdx;
			curType = D3DDEVTYPE_HAL;
			break; // stop looking
		} else if( devInfo->devType == D3DDEVTYPE_SW ) {
			currDevice = devIdx;
			curType = D3DDEVTYPE_SW;
			// but keep looking
		} else if( devInfo->devType == D3DDEVTYPE_REF && mAllowRef && curType != D3DDEVTYPE_SW ) {
			currDevice = devIdx;
			curType = D3DDEVTYPE_REF;
			// but keep looking
		}
	}
	if( currDevice == 0xffff )
		return D3DAPPERR_NOHARDWAREDEVICE;
	devInfo = &adapterInfo->devices[currDevice];
	
	devInfo->currMode = 0xffff;
	if( adapterInfo->userPrefWidth != 0 ) {
		// Try to find mode that matches user preference
		for( DWORD im = 0; im < devInfo->modeCount; im++ ) {
			modeInfo = &devInfo->modes[im];
			if( modeInfo->width == adapterInfo->userPrefWidth &&
				modeInfo->height == adapterInfo->userPrefHeight &&
				modeInfo->format == adapterInfo->userPrefFormat )
			{
				devInfo->currMode = im;
				break;
			}
		}
	}
	
	// If user-prefmErrored mode is not specified or not found,
	// use "Automatic" technique: 
	if( devInfo->currMode == 0xffff ) {
		if( devInfo->devType == D3DDEVTYPE_SW ) {
			// If using a SW rast then try to find a low resolution and 16-bpp.
			bool found16bit = false;			 
			DWORD smallestHeight = 0;
			devInfo->currMode = 0; // unless we find something better
			
			for( DWORD im = 0; im < devInfo->modeCount; im++ ) {
				modeInfo = &devInfo->modes[im];
				
				// Skip 640x400 because 640x480 is better :)
				if( modeInfo->height == 400 )
					continue; 
				
				if( modeInfo->height < smallestHeight || (modeInfo->height == smallestHeight && !found16bit) ) {
					smallestHeight = modeInfo->height;
					devInfo->currMode = im;
					found16bit = false;
					
					if( ( modeInfo->format == D3DFMT_R5G6B5 ||
						modeInfo->format == D3DFMT_X1R5G5B5 || 
						modeInfo->format == D3DFMT_A1R5G5B5 || 
						modeInfo->format == D3DFMT_A4R4G4B4 || 
						modeInfo->format == D3DFMT_X4R4G4B4 ) )
					{
						found16bit = true;
					}
				}
			}
		} else {
			// Try to find mode matching desktop resolution and 32-bpp.
			bool matchedSize = false;
			bool found32bit = false;
			devInfo->currMode = 0; // unless we find something better
			for( DWORD im = 0; im < devInfo->modeCount; im++ ) {
				modeInfo = &devInfo->modes[im];
				if( modeInfo->width == adapterInfo->desktopDM.Width &&
					modeInfo->height == adapterInfo->desktopDM.Height )
				{
					if( !matchedSize )
						devInfo->currMode = im;
					matchedSize = true;
					if( !found32bit &&
						( modeInfo->format == D3DFMT_X8R8G8B8 ||
						modeInfo->format == D3DFMT_A8R8G8B8 ) )
					{
						devInfo->currMode = im;
						found32bit = true;
						break;
					}
				}
			}
		}
	}
	
	// If desktop mode not found, pick highest mode available
	if( devInfo->currMode == 0xffff ) {
		DWORD maxWidth = 0;
		DWORD maxHeight = 0;
		DWORD maxBpp = 0;
		DWORD curWidth = 0;
		DWORD curHeight = 0;
		DWORD curBpp = 0;
		for( DWORD im = 0; im < devInfo->modeCount; im++ ) {
			modeInfo = &devInfo->modes[im];
			curWidth = modeInfo->width;
			curHeight = modeInfo->height;
			if( modeInfo->format == D3DFMT_X8R8G8B8 || modeInfo->format == D3DFMT_A8R8G8B8 ) {
				curBpp = 32;
			} else {
				curBpp = 16;
			}
			if( curWidth > maxWidth || curHeight > maxHeight || curWidth == maxWidth && curHeight == maxHeight && curBpp > maxBpp ) {
				maxWidth = curWidth;
				maxHeight = curHeight;
				maxBpp = curBpp;
				devInfo->currMode = im;
			}
		}
	}
	
	// Try to create the D3D device, falling back to lower-res modes if it fails
	bool atLeastOneFailure = false;
	for( ; ; ) {
		modeInfo = &devInfo->modes[devInfo->currMode];
		mCurrDevType = devInfo->devType;
		mCurrBehavior = modeInfo->behavior;
		ru->monIdx = monIdx;
		D3DPRESENT_PARAMETERS& rupp = mCurrPresentParams;
		rupp.BackBufferFormat = modeInfo->format;
		rupp.BackBufferWidth = modeInfo->width;
		rupp.BackBufferHeight = modeInfo->height;
		rupp.Windowed = false;
		rupp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		rupp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		rupp.AutoDepthStencilFormat = modeInfo->dsformat;
		rupp.BackBufferCount = 1;
		rupp.MultiSampleType = D3DMULTISAMPLE_NONE;
		rupp.SwapEffect = mSwapFxFullscreen;
		rupp.hDeviceWindow = adapterInfo->hwndDevice;
		rupp.EnableAutoDepthStencil = mUseDepthBuffer;
		rupp.Flags = 0;
		
		// create device
		hr = mD3D->CreateDevice( adapterIdx, mCurrDevType, 
			mHwnd, // (this is the focus window)
			mCurrBehavior, &rupp, 
			&mD3DDevice );
		if( SUCCEEDED( hr ) ) {
			// Give the client app an opportunity to reject this mode
			// due to not enough video memory, or any other reason
			if( SUCCEEDED( hr = confirmMode( mD3DDevice ) ) )
				break;
			else
				safeRelease( mD3DDevice );
		}
		
		// If we get here, remember that CreateDevice or confirmMode failed, so
		// we can change the default mode next time
		atLeastOneFailure = true;
		
		if( !findNextLowerMode( devInfo ) )
			break;
	}
	
	if( SUCCEEDED( hr ) && atLeastOneFailure && mRegistryPath[0] != TEXT('\0') ) {
		// Record the mode that succeeded in the registry so we can 
		// default to it next time
		TCHAR strKey[100];
		HKEY hkeyParent;
		HKEY hkey;
		
		adapterInfo->userPrefWidth = mCurrPresentParams.BackBufferWidth;
		adapterInfo->userPrefHeight = mCurrPresentParams.BackBufferHeight;
		adapterInfo->userPrefFormat = mCurrPresentParams.BackBufferFormat;
		
		if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, mRegistryPath, 
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyParent, NULL ) )
		{
			wsprintf( strKey, TEXT("Screen %d"), monIdx + 1 );
			if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
			{
				RegSetValueEx( hkey, TEXT("width"), NULL, REG_DWORD, 
					(BYTE*)&adapterInfo->userPrefWidth, sizeof(DWORD) );
				RegSetValueEx( hkey, TEXT("height"), NULL, REG_DWORD, 
					(BYTE*)&adapterInfo->userPrefHeight, sizeof(DWORD) );
				RegSetValueEx( hkey, TEXT("Format"), NULL, REG_DWORD, 
					(BYTE*)&adapterInfo->userPrefFormat, sizeof(DWORD) );
				RegSetValueEx( hkey, TEXT("Adapter ID"), NULL, REG_BINARY, 
					(BYTE*)&adapterInfo->adapterID.DeviceIdentifier, sizeof(GUID) );
				RegCloseKey( hkey );
			}
			RegCloseKey( hkeyParent );
		}
	}
	
	return hr;
}
*/


// --------------------------------------------------------------------------

/*
bool CD3DScreensaver::findNextLowerMode( SD3DDeviceInfo* devInfo )
{
	DWORD iModeCur = devInfo->currMode;
	SD3DModeInfo* pD3DModeInfoCur = &devInfo->modes[iModeCur];
	DWORD curWidth = pD3DModeInfoCur->width;
	DWORD curHeight = pD3DModeInfoCur->height;
	DWORD dwNumPixelsCur = curWidth * curHeight;
	D3DFORMAT d3dfmtCur = pD3DModeInfoCur->format;
	bool b32BitCur = (d3dfmtCur == D3DFMT_A8R8G8B8 ||
		d3dfmtCur == D3DFMT_X8R8G8B8);
	DWORD iModeNew;
	SD3DModeInfo* modeInfoNew;
	DWORD dwWidthNew;
	DWORD dwHeightNew;
	DWORD dwNumPixelsNew;
	D3DFORMAT d3dfmtNew = D3DFMT_UNKNOWN;
	bool b32BitNew;
	
	DWORD dwWidthBest = 0;
	DWORD dwHeightBest = 0;
	DWORD dwNumPixelsBest = 0;
	bool b32BitBest = false;
	DWORD iModeBest = 0xffff;
	
	for( iModeNew = 0; iModeNew < devInfo->modeCount; iModeNew++ ) {
		// Don't pick the same mode we currently have
		if( iModeNew == iModeCur )
			continue;
		
		// Get info about new mode
		modeInfoNew = &devInfo->modes[iModeNew];
		dwWidthNew = modeInfoNew->width;
		dwHeightNew = modeInfoNew->height;
		dwNumPixelsNew = dwWidthNew * dwHeightNew;
		d3dfmtNew = modeInfoNew->format;
		b32BitNew = (d3dfmtNew == D3DFMT_A8R8G8B8 ||
			d3dfmtNew == D3DFMT_X8R8G8B8);
		
		// If we're currently 32-bit and new mode is same width/height and 16-bit, take it
		if( b32BitCur && !b32BitNew && modeInfoNew->width == curWidth && modeInfoNew->height == curHeight ) {
			devInfo->currMode = iModeNew;
			return true;
		}
		
		// If new mode is smaller than current mode, see if it's our best so far
		if( dwNumPixelsNew < dwNumPixelsCur ) {
			// If current best is 32-bit, new mode needs to be bigger to be best
			if( b32BitBest && (dwNumPixelsNew < dwNumPixelsBest ) )
				continue;
			
			// If new mode is bigger or equal to best, make it the best
			if( (dwNumPixelsNew > dwNumPixelsBest) || (!b32BitBest && b32BitNew) ) {
				dwWidthBest = dwWidthNew;
				dwHeightBest = dwHeightNew;
				dwNumPixelsBest = dwNumPixelsNew;
				iModeBest = iModeNew;
				b32BitBest = b32BitNew;
			}
		}
	}
	if( iModeBest == 0xffff )
		return false; // no smaller mode found
	devInfo->currMode = iModeBest;
	return true;
}
*/

// --------------------------------------------------------------------------

/*
HRESULT CD3DScreensaver::createWindowedRenderUnit( SRenderUnit* ru )
{
	HRESULT hr;
	UINT adapterIdx = mCurrAdapterIdx;
	SD3DAdapterInfo* adapterInfo = mAdapters[adapterIdx];
	//DWORD monIdx = adapterInfo->monIdx;
	SD3DDeviceInfo* devInfo;
	D3DDEVTYPE curType;
	
	// Find the best device for the primary adapter.  Use HAL
	// if it's there, otherwise SW, otherwise REF.
	adapterInfo->currDevice = 0xffff; // unless we find something better
	curType = D3DDEVTYPE_FORCE_DWORD;
	for( DWORD devIdx = 0; devIdx < adapterInfo->deviceCount; devIdx++ ) {
		devInfo = &adapterInfo->devices[devIdx];
		if( devInfo->devType == D3DDEVTYPE_HAL && !adapterInfo->disableHAL && devInfo->canDoWindowed ) {
			adapterInfo->currDevice = devIdx;
			curType = D3DDEVTYPE_HAL;
			break;
		} else if( devInfo->devType == D3DDEVTYPE_SW && devInfo->canDoWindowed ) {
			adapterInfo->currDevice = devIdx;
			curType = D3DDEVTYPE_SW;
			// but keep looking
		} else if( devInfo->devType == D3DDEVTYPE_REF && mAllowRef && curType != D3DDEVTYPE_SW ) {
			adapterInfo->currDevice = devIdx;
			curType = D3DDEVTYPE_REF;
			// but keep looking
		}
	}
	if( adapterInfo->currDevice == 0xffff )
		return D3DAPPERR_NOHARDWAREDEVICE;
	devInfo = &adapterInfo->devices[adapterInfo->currDevice];
	
	SD3DWindowModeInfo SD3DWindowModeInfo;
	
	SD3DWindowModeInfo.dispFormat = adapterInfo->desktopDM.Format;
	SD3DWindowModeInfo.bbFormat = adapterInfo->desktopDM.Format;
	if( FAILED( checkWindowedFormat( adapterIdx, &SD3DWindowModeInfo ) ) ) {
		SD3DWindowModeInfo.bbFormat = D3DFMT_A8R8G8B8;
		if( FAILED( checkWindowedFormat( adapterIdx, &SD3DWindowModeInfo ) ) ) {
			SD3DWindowModeInfo.bbFormat = D3DFMT_X8R8G8B8;
			if( FAILED( checkWindowedFormat( adapterIdx, &SD3DWindowModeInfo ) ) ) {
				SD3DWindowModeInfo.bbFormat = D3DFMT_A1R5G5B5;
				if( FAILED( checkWindowedFormat( adapterIdx, &SD3DWindowModeInfo ) ) ) {
					SD3DWindowModeInfo.bbFormat = D3DFMT_R5G6B5;
					if( FAILED( checkWindowedFormat( adapterIdx, &SD3DWindowModeInfo ) ) ) {
						return E_FAIL;
					}
				}
			}
		}
	}
	
	mCurrDevType = devInfo->devType;
	mCurrBehavior = SD3DWindowModeInfo.behavior;
	ru->monIdx = monIdx;
	mCurrPresentParams.BackBufferWidth = 0;
	mCurrPresentParams.BackBufferHeight = 0;
	mCurrPresentParams.Windowed = true;
	mCurrPresentParams.FullScreen_RefreshRateInHz = 0;
	mCurrPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	mCurrPresentParams.BackBufferFormat = SD3DWindowModeInfo.bbFormat;
	mCurrPresentParams.AutoDepthStencilFormat = SD3DWindowModeInfo.dsformat;
	mCurrPresentParams.BackBufferCount = 1;
	mCurrPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	mCurrPresentParams.SwapEffect = mSwapFxWindowed;
	mCurrPresentParams.hDeviceWindow = adapterInfo->hwndDevice;
	mCurrPresentParams.EnableAutoDepthStencil = mUseDepthBuffer;
	mCurrPresentParams.Flags = 0;
	// create device
	hr = mD3D->CreateDevice( adapterIdx, mCurrDevType, mHwnd,
		mCurrBehavior, &mCurrPresentParams, &mD3DDevice );
	if ( FAILED(hr) ) {
		return hr;
	}
	
	return S_OK;
}
*/

// --------------------------------------------------------------------------

/*
// Store device description
void CD3DScreensaver::updateDeviceStats()
{
	DWORD iru;
	SRenderUnit* ru; 
	for( iru = 0; iru < mRenderUnitCount; iru++ ) {
		ru = &mRenderUnits[iru];
		if( mCurrDevType == D3DDEVTYPE_REF )
			lstrcpy( ru->deviceStats, TEXT("REF") );
		else if( mCurrDevType == D3DDEVTYPE_HAL )
			lstrcpy( ru->deviceStats, TEXT("HAL") );
		else if( mCurrDevType == D3DDEVTYPE_SW )
			lstrcpy( ru->deviceStats, TEXT("SW") );
		
		if( mCurrBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING && mCurrBehavior & D3DCREATE_PUREDEVICE ) {
			if( mCurrDevType == D3DDEVTYPE_HAL )
				lstrcat( ru->deviceStats, TEXT(" (pure hw vp)") );
			else
				lstrcat( ru->deviceStats, TEXT(" (simulated pure hw vp)") );
		} else if( mCurrBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) {
			if( mCurrDevType == D3DDEVTYPE_HAL )
				lstrcat( ru->deviceStats, TEXT(" (hw vp)") );
			else
				lstrcat( ru->deviceStats, TEXT(" (simulated hw vp)") );
		} else if( mCurrBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) {
			if( mCurrDevType == D3DDEVTYPE_HAL )
				lstrcat( ru->deviceStats, TEXT(" (mixed vp)") );
			else
				lstrcat( ru->deviceStats, TEXT(" (simulated mixed vp)") );
		} else if( mCurrBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING ) {
			lstrcat( ru->deviceStats, TEXT(" (sw vp)") );
		}
		
		if( mCurrDevType == D3DDEVTYPE_HAL ) {
			// Be sure not to overflow mDeviceStats when appending the adapter 
			// description, since it can be long.  Note that the adapter description
			// is initially CHAR and must be converted to TCHAR.
			lstrcat( ru->deviceStats, TEXT(": ") );
			const int cchDesc = sizeof(mAdapters[mCurrAdapterIdx]->adapterID.Description);
			TCHAR szDescription[cchDesc];
			convertAnsiStringToGenericCch( szDescription, 
				mAdapters[mCurrAdapterIdx]->adapterID.Description, cchDesc );
			lstrcat( ru->deviceStats, szDescription );
			int maxAppend = sizeof(mDeviceStats) / sizeof(TCHAR) -
				lstrlen( mDeviceStats ) - 1;
			_tcsncat( mDeviceStats, szDescription, maxAppend );
		}
	}
}
*/

// --------------------------------------------------------------------------

//  Updates internal variables and notifies client that we are switching
//	to a new SRenderUnit / D3D device.
/*
void CD3DScreensaver::switchToRenderUnit( UINT iru )
{
	//SRenderUnit* ru = &mRenderUnits[iru];
	//SMonitorInfo* monInfo = &mMonitors[ru->monIdx];
	
	mD3DDevice = mD3DDevice;
	if( !mWindowed )
		mRenderAreaCurr = monInfo->screenRect;
	
	if( mD3DDevice != NULL ) {
		// Store render target surface desc
		LPDIRECT3DSURFACE9 pBackBuffer;
		mD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		pBackBuffer->GetDesc( &mBackBufferDesc );
		pBackBuffer->Release();
	}
	
	lstrcpy( mDeviceStats, ru->deviceStats );
	lstrcpy( mFrameStats, ru->frameStats );
	
	// Notify the client to switch to this device
	setDevice( iru );
}
*/

// --------------------------------------------------------------------------
//  This function sets up an appropriate projection matrix to support 
//	rendering the appropriate parts of the scene to each screen.

/*
void CD3DScreensaver::buildProjMatrix( float nearPlane, float farPlane, D3DXMATRIX* matrix )
{
	D3DXMATRIX mat;
	INT cx, cy;
	INT dx, dy;
	INT dd;
	float l,r,t,b;
	
	cx = (mRenderAreaCurr.right + mRenderAreaCurr.left) / 2;
	cy = (mRenderAreaCurr.bottom + mRenderAreaCurr.top) / 2;
	dx = mRenderAreaCurr.right - mRenderAreaCurr.left;
	dy = mRenderAreaCurr.bottom - mRenderAreaCurr.top;
	
	dd = (dx > dy ? dy : dx);
	
	l = float(mRenderAreaCurr.left - cx) / (float)(dd);
	r = float(mRenderAreaCurr.right - cx) / (float)(dd);
	t = float(mRenderAreaCurr.top - cy) / (float)(dd);
	b = float(mRenderAreaCurr.bottom - cy) / (float)(dd);
	
	l = nearPlane * l;
	r = nearPlane * r;
	t = nearPlane * t;
	b = nearPlane * b;
	
	D3DXMatrixPerspectiveOffCenterLH( &mat, l, r, t, b, nearPlane, farPlane );
	*matrix = mat;
}


// --------------------------------------------------------------------------

//  This function sets up an appropriate projection matrix to support 
//	rendering the appropriate parts of the scene to each screen.
HRESULT CD3DScreensaver::setProjectionMatrix( float nearPlane, float farPlane )
{
	D3DXMATRIX mat;
	buildProjMatrix( nearPlane, farPlane, &mat );
	return mD3DDevice->SetTransform( D3DTS_PROJECTION, &mat );
}
*/

// --------------------------------------------------------------------------

/*
//  Callback function for sorting display modes (used by buildDeviceList).
static int __cdecl SortModesCallback( const void* arg1, const void* arg2 )
{
	const D3DDISPLAYMODE* p1 = (const D3DDISPLAYMODE*)arg1;
	const D3DDISPLAYMODE* p2 = (const D3DDISPLAYMODE*)arg2;
	if( p1->Width  < p2->Width )	return -1;
	if( p1->Width  > p2->Width )	return +1;
	if( p1->Height < p2->Height )	return -1;
	if( p1->Height > p2->Height )	return +1;
	if( p1->Format > p2->Format )	return -1;
	if( p1->Format < p2->Format )	return +1;
	return 0;
}
*/


// --------------------------------------------------------------------------

//  Builds a list of all available adapters, devices, and modes.
/*
HRESULT CD3DScreensaver::buildDeviceList()
{
	DWORD dwNumDeviceTypes;
	const TCHAR* strDeviceDescs[] = { TEXT("HAL"), TEXT("SW"), TEXT("REF") };
	const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
	if( mAllowRef )
		dwNumDeviceTypes = 3;
	else
		dwNumDeviceTypes = 2;
	
	HMONITOR hMonitor = NULL;
	bool bHALExists = false;
	bool bHALIsWindowedCompatible = false;
	bool bHALIsDesktopCompatible = false;
	bool bHALIsSampleCompatible = false;
	
	// Loop through all the adapters on the system (usually, there's just one
	// unless more than one graphics card is present).
	for( UINT adapterIdx = 0; adapterIdx < mD3D->GetAdapterCount(); adapterIdx++ ) {
		// Fill in adapter info
		if( mAdapters[mAdapterCount] == NULL ) {
			mAdapters[mAdapterCount] = new SD3DAdapterInfo;
			if( mAdapters[mAdapterCount] == NULL )
				return E_OUTOFMEMORY;
			ZeroMemory( mAdapters[mAdapterCount], sizeof(SD3DAdapterInfo) );
		}
		
		SD3DAdapterInfo* pAdapter  = mAdapters[mAdapterCount];
		mD3D->GetAdapterIdentifier( adapterIdx, 0, &pAdapter->adapterID );
		mD3D->GetAdapterDisplayMode( adapterIdx, &pAdapter->desktopDM );
		pAdapter->deviceCount	  = 0;
		pAdapter->currDevice = 0;
		pAdapter->leaveBlack = false;
		pAdapter->monIdx = NO_MONITOR;
		
		// Find the SMonitorInfo that corresponds to this adapter.  If the monitor
		// is disabled, the adapter has a NULL HMONITOR and we cannot find the 
		// corresponding SMonitorInfo.  (Well, if one monitor was disabled, we
		// could link the one SMonitorInfo with a NULL HMONITOR to the one
		// SD3DAdapterInfo with a NULL HMONITOR, but if there are more than one,
		// we can't link them, so it's safer not to ever try.)
		hMonitor = mD3D->GetAdapterMonitor( adapterIdx );
		if( hMonitor != NULL ) {
			for( DWORD monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
				SMonitorInfo* monInfo;
				monInfo = &mMonitors[monIdx];
				if( monInfo->hMonitor == hMonitor ) {
					pAdapter->monIdx = monIdx;
					monInfo->adapterIdx = adapterIdx;
					break;
				}
			}
		}
		
		// Enumerate all display modes on this adapter
		D3DDISPLAYMODE modes[100];
		D3DFORMAT	   allowedAdapterFmts[] = 
			{ D3DFMT_X8R8G8B8, D3DFMT_X1R5G5B5, D3DFMT_R5G6B5 };
		DWORD allowedAdapterFmtsCount = sizeof(allowedAdapterFmts) / sizeof(allowedAdapterFmts[0]);
		D3DFORMAT	adapterFmt;
		D3DFORMAT	formats[20];
		DWORD dwNumFormats		= 0;
		DWORD modeCount		= 0;
		DWORD dwNumAdapterModes = 0;
		
		// Add the adapter's current desktop format to the list of formats
		formats[dwNumFormats++] = pAdapter->desktopDM.Format;
		
		for( UINT iaaf = 0; iaaf < allowedAdapterFmtsCount; iaaf++ ) {
			adapterFmt = allowedAdapterFmts[iaaf];
			dwNumAdapterModes = mD3D->GetAdapterModeCount( adapterIdx, adapterFmt );
			for( UINT im = 0; im < dwNumAdapterModes; im++ ) {
				// Get the display mode attributes
				D3DDISPLAYMODE DisplayMode;
				mD3D->EnumAdapterModes( adapterIdx, adapterFmt, im, &DisplayMode );
				
				// Filter out low-resolution modes
				if( DisplayMode.Width  < 640 || DisplayMode.Height < 400 )
					continue;
				
				// Check if the mode already exists (to filter out refresh rates)
				for( DWORD m=0L; m<modeCount; m++ ) {
					if( ( modes[m].Width  == DisplayMode.Width	) &&
						( modes[m].Height == DisplayMode.Height ) &&
						( modes[m].Format == DisplayMode.Format ) )
						break;
				}
				
				// If we found a new mode, add it to the list of modes
				if( m == modeCount && modeCount < 99 ) {
					modes[modeCount].Width 	  = DisplayMode.Width;
					modes[modeCount].Height	  = DisplayMode.Height;
					modes[modeCount].Format	  = DisplayMode.Format;
					modes[modeCount].RefreshRate = 0;
					modeCount++;
					
					// Check if the mode's Format already exists
					for( DWORD f=0; f<dwNumFormats; f++ ) {
						if( DisplayMode.Format == formats[f] )
							break;
					}
					
					// If the Format is new, add it to the list
					if( f== dwNumFormats && dwNumFormats < 19)
						formats[dwNumFormats++] = DisplayMode.Format;
				}
			}
		}
		
		// Sort the list of display modes (by format, then width, then height)
		qsort( modes, modeCount, sizeof(D3DDISPLAYMODE), SortModesCallback );
		
		// Add devices to adapter
		for( UINT devIdx = 0; devIdx < dwNumDeviceTypes; devIdx++ ) {
			// Fill in device info
			SD3DDeviceInfo* pDevice;
			pDevice 				= &pAdapter->devices[pAdapter->deviceCount];
			pDevice->devType 	= DeviceTypes[devIdx];
			mD3D->GetDeviceCaps( adapterIdx, DeviceTypes[devIdx], &pDevice->caps );
			pDevice->desc		= strDeviceDescs[devIdx];
			pDevice->modeCount 	= 0;
			pDevice->currMode	= 0;
			pDevice->canDoWindowed = false;
			pDevice->windowed		= false;
			pDevice->multiSampleType = D3DMULTISAMPLE_NONE;
			
			// Examine each Format supported by the adapter to see if it will
			// work with this device and meets the needs of the application.
			bool  fmtConfirmed[20];
			DWORD behavior[20];
			D3DFORMAT fmtDepthStencil[20];
			
			for( DWORD f=0; f<dwNumFormats; f++ ) {
				fmtConfirmed[f] = false;
				fmtDepthStencil[f] = D3DFMT_UNKNOWN;
				
				// Skip formats that cannot be used as render targets on this device
				if( FAILED( mD3D->CheckDeviceType( adapterIdx, pDevice->devType,
					formats[f], formats[f], false ) ) )
					continue;
				
				if( pDevice->devType == D3DDEVTYPE_SW ) {
					// This system has a SW device
					pAdapter->hasSW = true;
				}
				
				if( pDevice->devType == D3DDEVTYPE_HAL ) {
					// This system has a HAL device
					bHALExists = true;
					pAdapter->hasHAL = true;
					
					// HAL can run in a window for some mode
					bHALIsWindowedCompatible = true;
					
					if( f == 0 ) {
						// HAL can run in a window for the current desktop mode
						bHALIsDesktopCompatible = true;
					}
				}
				
				// Confirm the device/Format for HW vertex processing
				if( pDevice->caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
					if( pDevice->caps.DevCaps&D3DDEVCAPS_PUREDEVICE ) {
						behavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
						if( SUCCEEDED( confirmDevice( &pDevice->caps, behavior[f], formats[f] ) ) )
							fmtConfirmed[f] = true;
					}
					
					if( false == fmtConfirmed[f] ) {
						behavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
						if( SUCCEEDED( confirmDevice( &pDevice->caps, behavior[f], formats[f] ) ) )
							fmtConfirmed[f] = true;
					}
					
					if( false == fmtConfirmed[f] ) {
						behavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;
						if( SUCCEEDED( confirmDevice( &pDevice->caps, behavior[f], formats[f] ) ) )
							fmtConfirmed[f] = true;
					}
				}
				
				// Confirm the device/Format for SW vertex processing
				if( false == fmtConfirmed[f] ) {
					behavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					if( SUCCEEDED( confirmDevice( &pDevice->caps, behavior[f], formats[f] ) ) )
						fmtConfirmed[f] = true;
				}
				
				if( fmtConfirmed[f] && mMultithreaded ) {
					behavior[f] |= D3DCREATE_MULTITHREADED;
				}
				
				// Find a suitable depth/stencil buffer Format for this device/Format
				if( fmtConfirmed[f] && mUseDepthBuffer ) {
					if( !findDepthStencilFormat( adapterIdx, pDevice->devType, formats[f], &fmtDepthStencil[f] ) ) {
						fmtConfirmed[f] = false;
					}
				}
			}
			
			// Add all enumerated display modes with confirmed formats to the
			// device's list of valid modes
			for( DWORD m=0L; m<modeCount; m++ ) {
				for( DWORD f=0; f<dwNumFormats; f++ ) {
					if( modes[m].Format == formats[f] ) {
						if( fmtConfirmed[f] == true ) {
							// Add this mode to the device's list of valid modes
							pDevice->modes[pDevice->modeCount].width	   = modes[m].Width;
							pDevice->modes[pDevice->modeCount].height	   = modes[m].Height;
							pDevice->modes[pDevice->modeCount].format	   = modes[m].Format;
							pDevice->modes[pDevice->modeCount].behavior = behavior[f];
							pDevice->modes[pDevice->modeCount].dsformat = fmtDepthStencil[f];
							pDevice->modeCount++;
							
							if( pDevice->devType == D3DDEVTYPE_HAL )
								bHALIsSampleCompatible = true;
						}
					}
				}
			}
			
			// Select any 640x480 mode for default (but prefer a 16-bit mode)
			for( m=0; m<pDevice->modeCount; m++ ) {
				if( pDevice->modes[m].width==640 && pDevice->modes[m].height==480 ) {
					pDevice->currMode = m;
					if( pDevice->modes[m].format == D3DFMT_R5G6B5 ||
						pDevice->modes[m].format == D3DFMT_X1R5G5B5 ||
						pDevice->modes[m].format == D3DFMT_A1R5G5B5 )
					{
						break;
					}
				}
			}
			
			// Check if the device is compatible with the desktop display mode
			// (which was added initially as formats[0])
			if( fmtConfirmed[0] ) {
				pDevice->canDoWindowed = true;
				pDevice->windowed		= true;
			}
			
			// If valid modes were found, keep this device
			if( pDevice->modeCount > 0 ) {
				pAdapter->deviceCount++;
				if( pDevice->devType == D3DDEVTYPE_SW )
					pAdapter->hasNeededSW = true;
				else if( pDevice->devType == D3DDEVTYPE_HAL )
					pAdapter->hasNeededHAL = true;
			}
		}
		
		// If valid devices were found, keep this adapter
		// Count adapters even if no devices, so we can throw up blank windows on them
		//		  if( pAdapter->deviceCount > 0 )
		mAdapterCount++;
	}

	return S_OK;
}
*/

// --------------------------------------------------------------------------

/*
HRESULT CD3DScreensaver::checkWindowedFormat( UINT adapterIdx, SD3DWindowModeInfo* windowModeInfo )
{
	HRESULT hr;
	SD3DAdapterInfo* adapterInfo = mAdapters[adapterIdx];
	SD3DDeviceInfo* devInfo = &adapterInfo->devices[adapterInfo->currDevice];
	bool fmtConfirmed = false;
	
	if( FAILED( hr = mD3D->CheckDeviceType( adapterIdx, devInfo->devType, adapterInfo->desktopDM.Format, windowModeInfo->bbFormat, true ) ) )
		return hr;
	
	// Confirm the device/Format for HW vertex processing
	if( devInfo->caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
		if( devInfo->caps.DevCaps&D3DDEVCAPS_PUREDEVICE ) {
			windowModeInfo->behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
			if( SUCCEEDED( confirmDevice( &devInfo->caps, windowModeInfo->behavior, windowModeInfo->bbFormat ) ) )
				fmtConfirmed = true;
		}
		
		if( !fmtConfirmed ) {
			windowModeInfo->behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			if( SUCCEEDED( confirmDevice( &devInfo->caps, windowModeInfo->behavior, windowModeInfo->bbFormat ) ) )
				fmtConfirmed = true;
		}
		
		if( !fmtConfirmed ) {
			windowModeInfo->behavior = D3DCREATE_MIXED_VERTEXPROCESSING;
			if( SUCCEEDED( confirmDevice( &devInfo->caps, windowModeInfo->behavior, windowModeInfo->bbFormat ) ) )
				fmtConfirmed = true;
		}
	}
	
	// Confirm the device/Format for SW vertex processing
	if( !fmtConfirmed ) {
		windowModeInfo->behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		if( SUCCEEDED( confirmDevice( &devInfo->caps, windowModeInfo->behavior, windowModeInfo->bbFormat ) ) )
			fmtConfirmed = true;
	}
	
	if( fmtConfirmed && mMultithreaded ) {
		windowModeInfo->behavior |= D3DCREATE_MULTITHREADED;
	}
	
	// Find a suitable depth/stencil buffer Format for this device/Format
	if( fmtConfirmed && mUseDepthBuffer ) {
		if( !findDepthStencilFormat( adapterIdx, devInfo->devType, windowModeInfo->bbFormat, &windowModeInfo->dsformat ) ) {
			fmtConfirmed = false;
		}
	}
	
	if( !fmtConfirmed )
		return E_FAIL;
	
	return S_OK;
}
*/

// --------------------------------------------------------------------------

//  Finds a depth/stencil Format for the given device that is compatible
//	with the render target Format and meets the needs of the app.
/*
bool CD3DScreensaver::findDepthStencilFormat( UINT adapterIdx, D3DDEVTYPE devType, D3DFORMAT targetFmt, D3DFORMAT* depthStencilFmt )
{
	if( mMinDepthBits <= 16 && mMinStencilBits == 0 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D16 ) ) ) {
				*depthStencilFmt = D3DFMT_D16;
				return true;
			}
		}
	}
	
	if( mMinDepthBits <= 15 && mMinStencilBits <= 1 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D15S1 ) ) ) {
				*depthStencilFmt = D3DFMT_D15S1;
				return true;
			}
		}
	}
	
	if( mMinDepthBits <= 24 && mMinStencilBits == 0 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D24X8 ) ) ) {
				*depthStencilFmt = D3DFMT_D24X8;
				return true;
			}
		}
	}
	
	if( mMinDepthBits <= 24 && mMinStencilBits <= 8 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D24S8 ) ) ) {
				*depthStencilFmt = D3DFMT_D24S8;
				return true;
			}
		}
	}
	
	if( mMinDepthBits <= 24 && mMinStencilBits <= 4 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D24X4S4 ) ) ) {
				*depthStencilFmt = D3DFMT_D24X4S4;
				return true;
			}
		}
	}
	
	if( mMinDepthBits <= 32 && mMinStencilBits == 0 ) {
		if( SUCCEEDED( mD3D->CheckDeviceFormat( adapterIdx, devType, targetFmt, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) ) {
			if( SUCCEEDED( mD3D->CheckDepthStencilMatch( adapterIdx, devType, targetFmt, targetFmt, D3DFMT_D32 ) ) ) {
				*depthStencilFmt = D3DFMT_D32;
				return true;
			}
		}
	}
	
	return false;
}
*/

// --------------------------------------------------------------------------

/*
void CD3DScreensaver::cleanup3DEnvironment()
{
	SRenderUnit* ru;
	
	for( DWORD iru = 0; iru < mRenderUnitCount; iru++ ) {
		ru = &mRenderUnits[iru];
		switchToRenderUnit( iru );
		if( mDevObjsRestored ) {
			passivateDeviceObjects();
			mDevObjsRestored = false;
		}
		if( mDevObjsInited ) {
			deleteDeviceObjects();
			mDevObjsInited = false;
		}
		safeRelease(mD3DDevice);
	}
	mRenderUnitCount = 0;
	safeRelease(mD3D);
}
*/


// --------------------------------------------------------------------------

/*
HRESULT CD3DScreensaver::render3DEnvironment()
{
	HRESULT hr;
	SRenderUnit* ru;
	SD3DAdapterInfo* adapterInfo;
	
	double appTime		= dingus::timer( TIMER_GETAPPTIME );
	double elapsedTime	= dingus::timer( TIMER_GETELAPSEDTIME );
	mTime		 = appTime;
	mElapsedTime = elapsedTime;
	
	
	// Tell client to update the world
	performLogic();
	updateFrameStats();
	
	for( DWORD iru = 0; iru < mRenderUnitCount; iru++ ) {
		ru = &mRenderUnits[iru];
		adapterInfo = mAdapters[mCurrAdapterIdx];
		
		switchToRenderUnit( iru );
		
		if( mD3DDevice == NULL )
			continue;
		
		// Test the cooperative level to see if it's okay to render
		if( FAILED( hr = mD3DDevice->TestCooperativeLevel() ) ) {
			// If the device was lost, do not render until we get it back
			if( D3DERR_DEVICELOST == hr )
				return S_OK;
			
			// Check if the device needs to be reset.
			if( D3DERR_DEVICENOTRESET == hr ) {
				// If we are windowed, read the desktop mode and use the same Format for
				// the back buffer
				if( mWindowed ) {
					mD3D->GetAdapterDisplayMode( mCurrAdapterIdx, &adapterInfo->desktopDM );
					// m_d3dpp.bbFormat = adapterInfo->desktopDM.Format;
				}
				
				if( mDevObjsRestored ) {
					passivateDeviceObjects();
					mDevObjsRestored = false;
				}
				if( FAILED( hr = mD3DDevice->Reset( &mCurrPresentParams ) ) ) {
					mErrorMode = true;
				} else {
					if( FAILED( hr = activateDeviceObjects() ) ) {
						mErrorMode = true;
					} else {
						mDevObjsRestored = true;
					}
				}
			}
		}
		
		// Tell client to render using the current device
		performRender();
	}
	
	// Call Present() in a separate loop once all rendering is done
	// so multiple monitors are as closely synced visually as possible
	for( iru = 0; iru < mRenderUnitCount; iru++ ) {
		ru = &mRenderUnits[iru];
		switchToRenderUnit( iru );
		// Present the results of the rendering to the screen
		mD3DDevice->Present( NULL, NULL, NULL, NULL );
	}
	
	return S_OK;
}
*/

// --------------------------------------------------------------------------

//  Update the box that shows the mErroror message
/*
void CD3DScreensaver::updateErrorBox()
{
	SMonitorInfo* monInfo;
	HWND hwnd;
	RECT rcBounds;
	static DWORD dwTimeLast = 0;
	DWORD dwTimeNow;
	float fTimeDelta;
	
	// Make sure all the RenderUnits / D3D devices have been torn down
	// so the mErroror box is visible
	if( mErrorMode && mRenderUnitCount > 0 ) {
		cleanup3DEnvironment();
	}
	
	// Update timing to determine how much to move mErroror box
	if( dwTimeLast == 0 )
		dwTimeLast = timeGetTime();
	dwTimeNow = timeGetTime();
	fTimeDelta = (float)(dwTimeNow - dwTimeLast) / 1000.0f;
	dwTimeLast = dwTimeNow;
	
	// Load mErroror string if necessary
	if( mErrorText[0] == TEXT('\0') ) {
		getTextForError( mErrorHR, mErrorText, sizeof(mErrorText) / sizeof(TCHAR) );
	}
	
	for( DWORD monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		hwnd = monInfo->hwnd;
		if( hwnd == NULL )
			continue;
		if( mSaverMode == SM_FULL ) {
			rcBounds = monInfo->screenRect;
			ScreenToClient( hwnd, (POINT*)&rcBounds.left );
			ScreenToClient( hwnd, (POINT*)&rcBounds.right );
		} else {
			rcBounds = mRenderAreaTotal;
		}
		
		if( monInfo->mErrorWidth == 0 ) {
			if( mSaverMode == SM_PREVIEW ) {
				monInfo->mErrorWidth = (float) (rcBounds.right - rcBounds.left);
				monInfo->mErrorHeight = (float) (rcBounds.bottom - rcBounds.top);
				monInfo->mErrorX = 0.0f;
				monInfo->mErrorY = 0.0f;
				monInfo->mErrorXVel = 0.0f;
				monInfo->mErrorYVel = 0.0f;
				InvalidateRect( hwnd, NULL, false );	// Invalidate the hwnd so it gets drawn
				UpdateWindow( hwnd );
			} else {
				monInfo->mErrorWidth = 300;
				monInfo->mErrorHeight = 150;
				monInfo->mErrorX = (rcBounds.right + rcBounds.left - monInfo->mErrorWidth) / 2.0f;
				monInfo->mErrorY = (rcBounds.bottom + rcBounds.top - monInfo->mErrorHeight) / 2.0f;
				monInfo->mErrorXVel = (rcBounds.right - rcBounds.left) / 10.0f;
				monInfo->mErrorYVel = (rcBounds.bottom - rcBounds.top) / 20.0f;
			}
		} else {
			if( mSaverMode != SM_PREVIEW ) {
				RECT rcOld;
				RECT rcNew;
				
				SetRect( &rcOld, (INT)monInfo->mErrorX, (INT)monInfo->mErrorY,
					(INT)(monInfo->mErrorX + monInfo->mErrorWidth),
					(INT)(monInfo->mErrorY + monInfo->mErrorHeight) );
				
				// Update rect velocity
				if( (monInfo->mErrorX + monInfo->mErrorXVel * fTimeDelta + 
					monInfo->mErrorWidth > rcBounds.right && monInfo->mErrorXVel > 0.0f) ||
					(monInfo->mErrorX + monInfo->mErrorXVel * fTimeDelta < 
					rcBounds.left && monInfo->mErrorXVel < 0.0f) )
				{
					monInfo->mErrorXVel = -monInfo->mErrorXVel;
				}
				if( (monInfo->mErrorY + monInfo->mErrorYVel * fTimeDelta + 
					monInfo->mErrorHeight > rcBounds.bottom && monInfo->mErrorYVel > 0.0f) ||
					(monInfo->mErrorY + monInfo->mErrorYVel * fTimeDelta < 
					rcBounds.top && monInfo->mErrorYVel < 0.0f) )
				{
					monInfo->mErrorYVel = -monInfo->mErrorYVel;
				}
				// Update rect position
				monInfo->mErrorX += monInfo->mErrorXVel * fTimeDelta;
				monInfo->mErrorY += monInfo->mErrorYVel * fTimeDelta;
				
				SetRect( &rcNew, (INT)monInfo->mErrorX, (INT)monInfo->mErrorY,
					(INT)(monInfo->mErrorX + monInfo->mErrorWidth),
					(INT)(monInfo->mErrorY + monInfo->mErrorHeight) );
				
				if( rcOld.left != rcNew.left || rcOld.top != rcNew.top ) {
					InvalidateRect( hwnd, &rcOld, false );	  // Invalidate old rect so it gets erased
					InvalidateRect( hwnd, &rcNew, false );	  // Invalidate new rect so it gets drawn
					UpdateWindow( hwnd );
				}
			}
		}
	}
}
*/

// --------------------------------------------------------------------------

//       Translate an HRESULT mErroror code into a string that can be displayed
//		 to explain the mErroror.	A class derived from CD3DScreensaver can 
//		 provide its own version of this function that provides app-specific
//		 mErroror translation instead of or in addition to calling this function.
//		 This function returns true if a specific mErroror was translated, or
//		 false if no specific translation for the HRESULT was found (though
//		 it still puts a generic string into pszError).
/*
bool CD3DScreensaver::getTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars )
{
	const DWORD dwErrorMap[][2] = {
		//	HRESULT, stringID
		(DWORD)E_FAIL, IDS_ERR_GENERIC,
		D3DAPPERR_NODIRECT3D, IDS_ERR_NODIRECT3D,
		D3DAPPERR_CREATEDEVICEFAILED, IDS_ERR_CREATEDEVICEFAILED,
		D3DAPPERR_NOCOMPATIBLEDEVICES, IDS_ERR_NOCOMPATIBLEDEVICES,
		D3DAPPERR_NOHARDWAREDEVICE, IDS_ERR_NOHARDWAREDEVICE,
		(DWORD)E_OUTOFMEMORY, IDS_ERR_OUTOFMEMORY,	   
		(DWORD)D3DERR_OUTOFVIDEOMEMORY, IDS_ERR_OUTOFVIDEOMEMORY,
		D3DAPPERR_NOPREVIEW, IDS_ERR_NOPREVIEW
	};
	const DWORD dwErrorMapSize = sizeof(dwErrorMap) / sizeof(DWORD[2]);
	
	DWORD iError;
	DWORD resid = 0;
	
	for( iError = 0; iError < dwErrorMapSize; iError++ ) {
		if( hr == (HRESULT)dwErrorMap[iError][0] )
			resid = dwErrorMap[iError][1];
	}
	if( resid == 0 )
		resid = IDS_ERR_GENERIC;
	
	LoadString( NULL, resid, pszError, dwNumChars );
	
	if( resid == IDS_ERR_GENERIC )
		return false;
	else
		return true;
}
*/

//-----------------------------------------------------------------------------

/*
void CD3DScreensaver::updateFrameStats()
{
	UINT iru;
	SRenderUnit* ru;
	UINT adapterIdx;
	static float fLastTime = 0.0f;
	static DWORD dwFrames  = 0L;
	float fTime = dingus::timer( TIMER_GETABSOLUTETIME );
	
	++dwFrames;
	
	// Update the scene stats once per second
	if( fTime - fLastTime > 1.0f ) {
		mFPS	  = dwFrames / (fTime - fLastTime);
		fLastTime = fTime;
		dwFrames  = 0L;
		
		for( iru = 0; iru < mRenderUnitCount; iru++ ) {
			ru = &mRenderUnits[iru];
			adapterIdx = mCurrAdapterIdx;
			
			// Get adapter's current mode so we can report
			// bit depth (back buffer depth may be unknown)
			D3DDISPLAYMODE mode;
			mD3D->GetAdapterDisplayMode( adapterIdx, &mode );
			
			_stprintf( ru->frameStats, TEXT("%.02f fps (%dx%dx%d)"), mFPS,
				mode.Width, mode.Height,
				mode.Format==D3DFMT_X8R8G8B8?32:16 );
			if( mUseDepthBuffer ) {
				SD3DAdapterInfo* adapterInfo = mAdapters[adapterIdx];
				SD3DDeviceInfo*	devInfo  = &adapterInfo->devices[adapterInfo->currDevice];
				SD3DModeInfo*	modeInfo	 = &devInfo->modes[devInfo->currMode];
				
				switch( modeInfo->dsformat ) {
				case D3DFMT_D16:
					lstrcat( ru->frameStats, TEXT(" (D16)") );
					break;
				case D3DFMT_D15S1:
					lstrcat( ru->frameStats, TEXT(" (D15S1)") );
					break;
				case D3DFMT_D24X8:
					lstrcat( ru->frameStats, TEXT(" (D24X8)") );
					break;
				case D3DFMT_D24S8:
					lstrcat( ru->frameStats, TEXT(" (D24S8)") );
					break;
				case D3DFMT_D24X4S4:
					lstrcat( ru->frameStats, TEXT(" (D24X4S4)") );
					break;
				case D3DFMT_D32:
					lstrcat( ru->frameStats, TEXT(" (D32)") );
					break;
				}
			}
		}
	}
}
*/

// --------------------------------------------------------------------------

/*
void CD3DScreensaver::doPaint(HWND hwnd, HDC hdc)
{
	HMONITOR hMonitor = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );
	SMonitorInfo* monInfo = NULL;
	for( DWORD monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		if( monInfo->hMonitor == hMonitor )
			break;
	}
	
	if( monIdx == mMonitorCount )
		return;
	
	// Draw the mErroror message box
	RECT rc;
	SetRect( &rc, (INT)monInfo->mErrorX, (INT)monInfo->mErrorY,
		(INT)(monInfo->mErrorX + monInfo->mErrorWidth),
		(INT)(monInfo->mErrorY + monInfo->mErrorHeight) );
	FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW+1));
	FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	RECT rc2;
	int height;
	rc2 = rc;
	height = DrawText(hdc, mErrorText, -1, &rc, DT_WORDBREAK | DT_CENTER | DT_CALCRECT );
	rc = rc2;
	
	rc2.top = (rc.bottom + rc.top - height) / 2;
	
	DrawText(hdc, mErrorText, -1, &rc2, DT_WORDBREAK | DT_CENTER );
	
	// Erase everywhere except the mErroror message box
	ExcludeClipRect( hdc, rc.left, rc.top, rc.right, rc.bottom );
	rc = monInfo->screenRect;
	ScreenToClient( hwnd, (POINT*)&rc.left );
	ScreenToClient( hwnd, (POINT*)&rc.right );
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH) );
}
*/

// --------------------------------------------------------------------------
/*
void CD3DScreensaver::changePassword()
{
	// Load the password change DLL
	HINSTANCE mpr = LoadLibrary( TEXT("MPR.DLL") );
	
	if ( mpr != NULL ) {
		// Grab the password change function from it
		typedef DWORD (PASCAL *PWCHGPROC)( LPCSTR, HWND, DWORD, LPVOID );
		PWCHGPROC pwd = (PWCHGPROC)GetProcAddress( mpr, "PwdChangePasswordA" );
		
		// Do the password change
		if ( pwd != NULL )
			pwd( "SCRSAVE", mHwndParent, 0, NULL );
		
		// Free the library
		FreeLibrary( mpr );
	}
}
*/

// --------------------------------------------------------------------------

/*
// Displays error messages in a message box
HRESULT CD3DScreensaver::displayErrorMsg( HRESULT hr, DWORD type )
{
	TCHAR strMsg[512];
	getTextForError( hr, strMsg, 512 );
	MessageBox( mHwnd, strMsg, mWindowTitle, MB_ICONERROR | MB_OK );
	return hr;
}
*/


// --------------------------------------------------------------------------

/*
//  Read the registry settings that affect how the screens are set up and used.
void CD3DScreensaver::readScreenSettings( HKEY hkeyParent )
{
	TCHAR strKey[100];
	//DWORD monIdx;
	SMonitorInfo* monInfo;
	DWORD adapterIdx;
	SD3DAdapterInfo* adapterInfo;
	HKEY hkey;
	DWORD type = REG_DWORD;
	DWORD dwLength;
	GUID guidAdapterID;
	GUID guidZero;
	ZeroMemory( &guidAdapterID, sizeof(GUID) );
	ZeroMemory( &guidZero, sizeof(GUID) );
	
	dwLength = sizeof(DWORD);
	for( monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		adapterIdx = monInfo->adapterIdx;
		if( adapterIdx == NO_ADAPTER )
			continue; 
		adapterInfo = mAdapters[adapterIdx];
		wsprintf( strKey, TEXT("Screen %d"), monIdx + 1 );
		if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
		{
			dwLength = sizeof(GUID);
			RegQueryValueEx( hkey, TEXT("Adapter ID"), NULL, &type, 
				(BYTE*)&guidAdapterID, &dwLength);
			
			dwLength = sizeof(DWORD);
			RegQueryValueEx( hkey, TEXT("Leave Black"), NULL, &type, 
				(BYTE*)&adapterInfo->leaveBlack, &dwLength);
			
			if( guidAdapterID == adapterInfo->adapterID.DeviceIdentifier ||
				guidAdapterID == guidZero )
			{
				dwLength = sizeof(DWORD);
				RegQueryValueEx( hkey, TEXT("Disable Hardware"), NULL, &type, 
					(BYTE*)&adapterInfo->disableHAL, &dwLength);
				dwLength = sizeof(DWORD);
				RegQueryValueEx( hkey, TEXT("width"), NULL, &type, 
					(BYTE*)&adapterInfo->userPrefWidth, &dwLength);
				dwLength = sizeof(DWORD);
				RegQueryValueEx( hkey, TEXT("height"), NULL, &type, 
					(BYTE*)&adapterInfo->userPrefHeight, &dwLength);
				dwLength = sizeof(DWORD);
				RegQueryValueEx( hkey, TEXT("Format"), NULL, &type, 
					(BYTE*)&adapterInfo->userPrefFormat, &dwLength);
			}
			RegCloseKey( hkey);
		}
	}
}
*/

// --------------------------------------------------------------------------

//  Write the registry settings that affect how the screens are set up and used.
void CD3DScreensaver::writeScreenSettings( HKEY hkeyParent )
{
	TCHAR strKey[100];
	//DWORD monIdx;
	SMonitorInfo* monInfo;
	DWORD adapterIdx;
	SD3DAdapterInfo* adapterInfo;
	HKEY hkey;
	
	for( monIdx = 0; monIdx < mMonitorCount; monIdx++ ) {
		monInfo = &mMonitors[monIdx];
		adapterIdx = monInfo->adapterIdx;
		if( adapterIdx == NO_ADAPTER )
			continue; 
		adapterInfo = mAdapters[adapterIdx];
		wsprintf( strKey, TEXT("Screen %d"), monIdx + 1 );
		if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
		{
			RegSetValueEx( hkey, TEXT("Leave Black"), NULL, REG_DWORD, 
				(BYTE*)&adapterInfo->leaveBlack, sizeof(DWORD) );
			RegSetValueEx( hkey, TEXT("Disable Hardware"), NULL, REG_DWORD, 
				(BYTE*)&adapterInfo->disableHAL, sizeof(DWORD) );
			RegSetValueEx( hkey, TEXT("width"), NULL, REG_DWORD, 
				(BYTE*)&adapterInfo->userPrefWidth, sizeof(DWORD) );
			RegSetValueEx( hkey, TEXT("height"), NULL, REG_DWORD, 
				(BYTE*)&adapterInfo->userPrefHeight, sizeof(DWORD) );
			RegSetValueEx( hkey, TEXT("Format"), NULL, REG_DWORD, 
				(BYTE*)&adapterInfo->userPrefFormat, sizeof(DWORD) );
			RegSetValueEx( hkey, TEXT("Adapter ID"), NULL, REG_BINARY, 
				(BYTE*)&adapterInfo->adapterID.DeviceIdentifier, sizeof(GUID) );
			RegCloseKey( hkey);
		}
	}
}


// --------------------------------------------------------------------------

/*
void CD3DScreensaver::doScreenSettingsDlg( HWND hwndParent )
{
	LPCTSTR pstrTemplate = MAKEINTRESOURCE( IDD_SINGLEMONITORSETTINGS );
	DialogBox(mInstance, pstrTemplate, hwndParent, screenSettingsDlgProcStub );
}


// --------------------------------------------------------------------------

INT_PTR CALLBACK CD3DScreensaver::screenSettingsDlgProcStub( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return gD3DScreensaver->screenSettingsDlgProc( hwnd, msg, wParam, lParam );
}


// We need to store a copy of the original screen settings so that the user
// can modify those settings in the dialog, then hit Cancel and have the
// original settings restored.
static SD3DAdapterInfo* s_AdaptersSave[9];


INT_PTR CD3DScreensaver::screenSettingsDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND hwndModeList = GetDlgItem(hwnd, IDC_MODESCOMBO);
	//DWORD monIdx;
	SMonitorInfo* monInfo;
	DWORD adapterIdx;
	
	switch( msg ) {
	case WM_INITDIALOG:
		{
			for( adapterIdx = 0; adapterIdx < mAdapterCount; adapterIdx++ ) {
				s_AdaptersSave[adapterIdx] = new SD3DAdapterInfo;
				if( s_AdaptersSave[adapterIdx] != NULL )
					*s_AdaptersSave[adapterIdx] = *mAdapters[adapterIdx];
			}
			setupAdapterPage( hwnd );
		}
		return true;
		
	case WM_COMMAND:
		switch( LOWORD( wParam ) ) {
			
		case IDC_MODESCOMBO:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				DWORD iSel;
				DWORD im;
				
				getBestAdapter( &adapterIdx );
				monIdx = mAdapters[adapterIdx]->monIdx;
				monInfo = &mMonitors[monIdx];
				adapterIdx = monInfo->adapterIdx;
				iSel = ComboBox_GetCurSel( hwndModeList );
				if( iSel == 0 ) {
					// "Automatic"
					mAdapters[adapterIdx]->userPrefWidth = 0;
					mAdapters[adapterIdx]->userPrefHeight = 0;
					mAdapters[adapterIdx]->userPrefFormat = D3DFMT_UNKNOWN;
				} else {
					SD3DAdapterInfo* adapterInfo = mAdapters[adapterIdx];
					SD3DDeviceInfo* devInfo;
					SD3DModeInfo* modeInfo;
					devInfo = &adapterInfo->devices[adapterInfo->currDevice];
					im = (DWORD)ComboBox_GetItemData( hwndModeList, iSel );
					modeInfo = &devInfo->modes[im];
					mAdapters[adapterIdx]->userPrefWidth = modeInfo->width;
					mAdapters[adapterIdx]->userPrefHeight = modeInfo->height;
					mAdapters[adapterIdx]->userPrefFormat = modeInfo->format;
				}
			}
			break;
			
		case IDC_DISABLEHW:
			getBestAdapter( &adapterIdx );
			monIdx = mAdapters[adapterIdx]->monIdx;
			monInfo = &mMonitors[monIdx];
			adapterIdx = monInfo->adapterIdx;
			if( IsDlgButtonChecked( hwnd, IDC_DISABLEHW ) == BST_CHECKED )
				mAdapters[adapterIdx]->disableHAL = true;
			else
				mAdapters[adapterIdx]->disableHAL = false;
			setupAdapterPage( hwnd );
			break;
			
		case IDC_MOREINFO:
			{
				getBestAdapter( &adapterIdx );
				monIdx = mAdapters[adapterIdx]->monIdx;
				monInfo = &mMonitors[monIdx];
				adapterIdx = monInfo->adapterIdx;
				SD3DAdapterInfo* adapterInfo;
				TCHAR szText[500];
				
				if( monInfo->hMonitor == NULL )
					adapterInfo = NULL;
				else
					adapterInfo = mAdapters[monInfo->adapterIdx];
				
				// Accelerated / Unaccelerated settings
				bool hasHAL = false;
				bool hasNeededHAL = false;
				bool bDisabledHAL = false;
				bool hasSW = false;
				bool hasNeededSW = false;
				
				if( adapterInfo != NULL ) {
					hasHAL = adapterInfo->hasHAL;
					hasNeededHAL = adapterInfo->hasNeededHAL;
					bDisabledHAL = adapterInfo->disableHAL;
					hasSW = adapterInfo->hasSW;
					hasNeededSW = adapterInfo->hasNeededSW;
				}
				if( hasHAL && !bDisabledHAL && hasNeededHAL ) {
					// Good HAL
					LoadString( NULL, IDS_INFO_GOODHAL, szText, 500 );
				} else if( hasHAL && bDisabledHAL ) {
					// Disabled HAL
					if( hasSW && hasNeededSW )
						LoadString( NULL, IDS_INFO_DISABLEDHAL_GOODSW, szText, 500 );
					else if( hasSW )
						LoadString( NULL, IDS_INFO_DISABLEDHAL_BADSW, szText, 500 );
					else 
						LoadString( NULL, IDS_INFO_DISABLEDHAL_NOSW, szText, 500 );
				} else if( hasHAL && !hasNeededHAL ) {
					// Bad HAL
					if( hasSW && hasNeededSW )
						LoadString( NULL, IDS_INFO_BADHAL_GOODSW, szText, 500 );
					else if( hasSW )
						LoadString( NULL, IDS_INFO_BADHAL_BADSW, szText, 500 );
					else 
						LoadString( NULL, IDS_INFO_BADHAL_NOSW, szText, 500 );
				} else  {
					// No HAL
					if( hasSW && hasNeededSW )
						LoadString( NULL, IDS_INFO_NOHAL_GOODSW, szText, 500 );
					else if( hasSW  )
						LoadString( NULL, IDS_INFO_NOHAL_BADSW, szText, 500 );
					else 
						LoadString( NULL, IDS_INFO_NOHAL_NOSW, szText, 500 );
				}
				
				MessageBox( hwnd, szText, monInfo->deviceName, MB_OK | MB_ICONINFORMATION );
				break;
			}
			
		case IDOK:
			for( adapterIdx = 0; adapterIdx < mAdapterCount; adapterIdx++ ) {
				safeDelete( s_AdaptersSave[adapterIdx] );
			}
			EndDialog(hwnd, IDOK);
			break;
			
		case IDCANCEL:
			// Restore member values to original state
			for( adapterIdx = 0; adapterIdx < mAdapterCount; adapterIdx++ ) {
				if( s_AdaptersSave[adapterIdx] != NULL )
					*mAdapters[adapterIdx] = *s_AdaptersSave[adapterIdx];
				safeDelete( s_AdaptersSave[adapterIdx] );
			}
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		return true;
		
	default:
		return false;
	}
}

*/

// --------------------------------------------------------------------------

/*
//  Set up the controls for a given page in the Screen Settings dialog.
void CD3DScreensaver::setupAdapterPage( HWND hwnd )
{
	HWND hwndModeList = GetDlgItem(hwnd, IDC_MODESCOMBO);
	HWND hwndDesc = GetDlgItem(hwnd, IDC_ADAPTERNAME);
	SMonitorInfo* monInfo;
	SD3DAdapterInfo* adapterInfo;
	SD3DDeviceInfo* devInfo;
	SD3DModeInfo* modeInfo;
	
	DWORD adapterIdx;
	//DWORD monIdx = 0;

	getBestAdapter( &adapterIdx );
	if( adapterIdx != NO_ADAPTER ) {
		adapterInfo = mAdapters[adapterIdx];
		monIdx = adapterInfo->monIdx;
	}
	
	monInfo = &mMonitors[monIdx];
	
	SetWindowText( hwndDesc, monInfo->deviceName );
	
	if( monInfo->adapterIdx == NO_ADAPTER )
		adapterInfo = NULL;
	else
		adapterInfo = mAdapters[monInfo->adapterIdx];
	
	// Accelerated / Unaccelerated settings
	bool hasHAL = false;
	bool hasNeededHAL = false;
	bool bDisabledHAL = false;
	bool hasSW = false;
	bool hasNeededSW = false;
	
	if( adapterInfo != NULL ) {
		hasHAL = adapterInfo->hasHAL;
		hasNeededHAL = adapterInfo->hasNeededHAL;
		bDisabledHAL = adapterInfo->disableHAL;
		hasSW = adapterInfo->hasSW;
		hasNeededSW = adapterInfo->hasNeededSW;
	}
	
	TCHAR szStatus[200];
	if( hasHAL && !bDisabledHAL && hasNeededHAL ) {
		LoadString( NULL, IDS_RENDERING_HAL, szStatus, 200 );
	} else if( hasSW && hasNeededSW ) {
		LoadString( NULL, IDS_RENDERING_SW, szStatus, 200 );
	} else {
		LoadString( NULL, IDS_RENDERING_NONE, szStatus, 200 );
	}
	SetWindowText( GetDlgItem( hwnd, IDC_RENDERING ), szStatus );
	
	if( hasHAL && hasNeededHAL ) {
		EnableWindow( GetDlgItem( hwnd, IDC_DISABLEHW ), true );
		CheckDlgButton( hwnd, IDC_DISABLEHW, 
			adapterInfo->disableHAL ? BST_CHECKED : BST_UNCHECKED );
	} else {
		EnableWindow( GetDlgItem( hwnd, IDC_DISABLEHW ), false );
		CheckDlgButton( hwnd, IDC_DISABLEHW, BST_UNCHECKED );
	}
	
	if( ( hasNeededHAL && !bDisabledHAL ) || hasNeededSW ) {
		EnableWindow(GetDlgItem(hwnd, IDC_SCREENUSAGEBOX), true);
	} else {
		EnableWindow(GetDlgItem(hwnd, IDC_SCREENUSAGEBOX), false);
	}
	
	// Mode list
	ComboBox_ResetContent( hwndModeList );
	if( adapterInfo == NULL )
		return;
	TCHAR strAutomatic[100];
	GetWindowText(GetDlgItem(hwnd, IDC_AUTOMATIC), strAutomatic, 100);
	ComboBox_AddString( hwndModeList, strAutomatic );
	ComboBox_SetItemData( hwndModeList, 0, -1 );
	devInfo = &adapterInfo->devices[adapterInfo->currDevice];
	DWORD iSelInitial = 0;
	TCHAR strModeFmt[100];
	
	GetWindowText(GetDlgItem(hwnd, IDC_MODEFMT), strModeFmt, 100);
	for( DWORD im = 0; im < devInfo->modeCount; im++ ) {
		DWORD dwBitDepth;
		TCHAR strMode[80];
		DWORD dwItem;
		
		modeInfo = &devInfo->modes[im];
		dwBitDepth = 16;
		if( modeInfo->format == D3DFMT_X8R8G8B8 ||
			modeInfo->format == D3DFMT_A8R8G8B8 ||
			modeInfo->format == D3DFMT_R8G8B8 )
		{
			dwBitDepth = 32;
		}
		
		wsprintf( strMode, strModeFmt, modeInfo->width,
			modeInfo->height, dwBitDepth );
		dwItem = ComboBox_AddString( hwndModeList, strMode );
		ComboBox_SetItemData( hwndModeList, dwItem, im );
		
		if( modeInfo->width == adapterInfo->userPrefWidth &&
			modeInfo->height == adapterInfo->userPrefHeight &&
			modeInfo->format == adapterInfo->userPrefFormat )
		{
			iSelInitial = dwItem;
		}
	}
	ComboBox_SetCurSel( hwndModeList, iSelInitial );
}
*/
