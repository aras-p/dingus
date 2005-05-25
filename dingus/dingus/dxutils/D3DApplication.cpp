// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include <windowsx.h>
#include "D3DApplication.h"
#include "DXUtil.h"
#include "D3DUtil.h"
#include "resource.h"
#include <regstr.h>

#include "../kernel/D3DDevice.h"

using namespace dingus;


//---------------------------------------------------------------------------
// global access to the app (needed for the global gWndProc)

static CD3DApplication* gD3DApp = NULL;

//---------------------------------------------------------------------------
// Static msg handler which passes messages to the application class.

LRESULT CALLBACK gWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return gD3DApp->msgProc( hWnd, uMsg, wParam, lParam );
}


//---------------------------------------------------------------------------
// CD3DApplication
//---------------------------------------------------------------------------

CD3DApplication::CD3DApplication()
:	mD3D(NULL), mD3DDevice(NULL),
	mHWnd(NULL), mHWndFocus(NULL), mHMenu(NULL),
	mWindowed(true), mActive(false), mDeviceLost(false),
	mMinimized(false), mMaximized(false), mIgnoreSizeChange(false),
	mDeviceObjectsInited(false), mDeviceObjectsRestored(false),
	mCreateFlags(0),
	mFPS(0.0f),
	mWindowTitle( _T("D3D9 Application")),
	mCreationWidth(400),
	mCreationHeight(300),
	mShowCursorWhenFullscreen(false), mStartFullscreen(false), mVSyncFullscreen(true),
	mSelectDeviceAtStartup(false),
	mDebugTimer(false),

	mSSInStartingPause(false),
	mSSCheckingPassword(false),
	mSSIsWin9x(false),
	mSSMouseMoveCount(0),
	mSSHwndParent(0), mSSPasswordDLL(0), mSSVerifyPasswordProc(0)
{
	gD3DApp 		  = this;
	
	mDeviceStats[0] = _T('\0');
	mFrameStats[0]	= _T('\0');
	mSSRegistryPath[0] = _T('\0');
	
	pause( true ); // pause until we're ready to render
	
	// When mClipCursorWhenFullscreen is true, the cursor is limited to
	// the device window when the app goes fullscreen.	This prevents users
	// from accidentally clicking outside the app window on a multimon system.
	// This flag is turned off by default for debug builds, since it makes 
	// multimon debugging difficult.
#if defined(_DEBUG) || defined(DEBUG)
	mClipCursorWhenFullscreen = false;
#else
	mClipCursorWhenFullscreen = true;
#endif
}



bool CD3DApplication::checkDeviceHelper( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	return gD3DApp->checkDevice( caps, vproc, errors );
}


HRESULT CD3DApplication::create( HINSTANCE hInstance, bool screenSaverMode )
{
	HRESULT hr;

	// if we're screensaver - set low priority and parse cmd line
	if( screenSaverMode ) {
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );
		mSSMode = ssParseCmdLine( GetCommandLine() );
		// force windowed in preview mode
		if( mSSMode == SM_PREVIEW )
			mStartFullscreen = false;
	} else {
		mSSMode = SM_NONE;
		mSSHwndParent = NULL;
	}
	
	mHInstance = hInstance;
	
	//
	// setup d3d object, enumerate possible options, etc.

	// create the Direct3D object
	mD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if( mD3D == NULL )
		return displayErrorMsg( (HRESULT)NODIRECT3D, APPMUSTEXIT );
	
	// Build a list of Direct3D adapters, modes and devices. The
	// checkDevice() callback is used to confirm that only devices that
	// meet the app's requirements are considered.
	mEnumeration.setDirect3D( *mD3D );
	mEnumeration.mConfirmDeviceCallback = checkDeviceHelper;

	if( FAILED( hr = mEnumeration.enumerate() ) ) {
		safeRelease( mD3D );
		return displayErrorMsg( hr, APPMUSTEXIT );
	}

	if( FAILED( hr = chooseInitialD3DSettings() ) ) {
		safeRelease( mD3D );
		return displayErrorMsg( hr, APPMUSTEXIT );
	}
	
	//
	// manage screensaver running modes

	switch( mSSMode ) {
	case SM_CONFIG:
		ssDoConfig();
		safeRelease( mD3D );
		return S_OK;
		break;
	case SM_PASSCHANGE:
		ssChangePassword();
		safeRelease( mD3D );
		return S_OK;
		break;
	}
	
	
	// force no dialog at startup if we're screensaver
	if( mSSMode != SM_NONE )
		mSelectDeviceAtStartup = false;

	if( mSelectDeviceAtStartup ) {
		bool ok = justShowSettingsDialog();
		if( !ok ) {
			safeRelease( mD3D );
			if( mHWnd )
				SendMessage( mHWnd, WM_CLOSE, 0, 0 );
			return E_FAIL;
		}
	}

	//
	// create main window

	// Unless a substitute hWnd has been specified, create a window to
	// render into, OR
	// do the needed thing if we're screensaver
	if( mSSMode==SM_NONE && mHWnd == NULL ||
		(mSSMode==SM_PREVIEW || mSSMode==SM_TEST || mSSMode==SM_FULL) )
	{
		// Register the windows class
		WNDCLASS wndClass = { 0, gWndProc, 0, 0, hInstance,
			LoadIcon( hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON) ),
			LoadCursor( NULL, IDC_ARROW ),
			(HBRUSH)GetStockObject(WHITE_BRUSH),
			NULL, _T("D3D Window")
		};
		// TBD - need this?
		//if( screenSaverMode )
		//	wndClass.style = CS_VREDRAW | CS_HREDRAW;
		RegisterClass( &wndClass );
		
		// Set the window's initial style and size
		RECT rc;
		if( mSSMode==SM_PREVIEW ) {
			GetClientRect( mSSHwndParent, &rc );
			mWindowStyle = WS_VISIBLE | WS_CHILD;
			AdjustWindowRect( &rc, mWindowStyle, false );
			// create the render window
			mHWnd = CreateWindow( _T("D3D Window"), mWindowTitle, mWindowStyle,
				CW_USEDEFAULT, CW_USEDEFAULT,
				(rc.right-rc.left), (rc.bottom-rc.top), mSSHwndParent,
				NULL, hInstance, 0 );
		} else {
			mWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
				WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
			SetRect( &rc, 0, 0, mCreationWidth, mCreationHeight );
			AdjustWindowRect( &rc, mWindowStyle, true );
			// create the render window
			mHWnd = CreateWindow( _T("D3D Window"), mWindowTitle, mWindowStyle,
				CW_USEDEFAULT, CW_USEDEFAULT,
				(rc.right-rc.left), (rc.bottom-rc.top), NULL,
				LoadMenu( hInstance, NULL ),
				hInstance, 0 );
		}
	}
	
	// The focus window can be a specified to be a different window than the
	// device window. If not, use the device window as the focus window.
	if( mHWndFocus == NULL )
		mHWndFocus = mHWnd;
	
	// Save window properties
	mWindowStyle = GetWindowLong( mHWnd, GWL_STYLE );
	GetWindowRect( mHWnd, &mWindowBounds );
	GetClientRect( mHWnd, &mWindowClient );

	// In screensaver preview mode, "pause" (enter a limited message loop)
	// briefly before proceeding, so the display control panel knows to
	// update itself.
	if( mSSMode == SM_PREVIEW ) {
		mSSInStartingPause = true;
		// Post a message to mark the end of the initial group of window messages
		PostMessage( mHWnd, WM_USER, 0, 0 );
		MSG msg;
		while( mSSInStartingPause ) {
			if( !GetMessage( &msg, mHWnd, 0, 0 ) ) {
				PostQuitMessage(0);
				break;
			}
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	// read SS settings
	if( mSSMode != SM_NONE )
		ssReadSettings();
	
	
	// Initialize the application timer
	CSystemTimer::getInstance().reset();
	CSystemTimer::getInstance().start();
	
	// Initialize the app
	if( FAILED( hr = initialize() ) ) {
		safeRelease( mD3D );
		return displayErrorMsg( hr, APPMUSTEXIT );
	}
	
	// Initialize the 3D environment for the app
	if( FAILED( hr = initialize3DEnvironment() ) ) {
		safeRelease( mD3D );
		return displayErrorMsg( hr, APPMUSTEXIT );
	}
	
	// The app is ready to go
	pause( false );
	
	return S_OK;
}



/**
 *  Sets up mSettings with best available windowed mode, subject to the
 *  requireHAL and requireREF constraints. Returns false if no such mode
 *  can be found.
 */
bool CD3DApplication::findBestWindowedMode( bool requireHAL, bool requireREF )
{
	// Get display mode of primary adapter (which is assumed to be where the
	// window will appear)
	D3DDISPLAYMODE primaryDesktopDM;
	mD3D->GetAdapterDisplayMode( 0, &primaryDesktopDM );
	
	const SD3DAdapterInfo* bestAdapterInfo = NULL;
	const SD3DDeviceInfo* bestDeviceInfo = NULL;
	const SD3DDeviceCombo* bestDeviceCombo = NULL;
	
	for( int iai = 0; iai < mEnumeration.mAdapterInfos.size(); ++iai ) {
		const SD3DAdapterInfo* adInfo = mEnumeration.mAdapterInfos[iai];
		for( int idi = 0; idi < adInfo->deviceInfos.size(); ++idi ) {
			const SD3DDeviceInfo* devInfo = adInfo->deviceInfos[idi];
			if( requireHAL && devInfo->caps.getDeviceType() != D3DDEVTYPE_HAL )
				continue;
			if( requireREF && devInfo->caps.getDeviceType() != D3DDEVTYPE_REF )
				continue;
			for( int idc = 0; idc < devInfo->deviceCombos.size(); ++idc ) {
				const SD3DDeviceCombo* devCombo = devInfo->deviceCombos[idc];
				bool matchesBB = (devCombo->backBufferFormat == devCombo->adapterFormat);
				if( !devCombo->isWindowed )
					continue;
				if( devCombo->adapterFormat != primaryDesktopDM.Format )
					continue;
				// If we haven't found a compatible DeviceCombo yet, or if
				// this set is better (because it's a HAL, and/or because
				// formats match better), save it
				if( !bestDeviceCombo || 
					bestDeviceCombo->deviceType != D3DDEVTYPE_HAL && devCombo->deviceType == D3DDEVTYPE_HAL ||
					devCombo->deviceType == D3DDEVTYPE_HAL && matchesBB )
				{
					bestAdapterInfo = adInfo;
					bestDeviceInfo = devInfo;
					bestDeviceCombo = devCombo;
					if( devCombo->deviceType == D3DDEVTYPE_HAL && matchesBB ) {
						// This windowed device combo looks great -- take it
						goto _endComboSearch;
					}
					// Otherwise keep looking for better combo
				}
			}
		}
	}

_endComboSearch:
	CD3DSettings::SSettings& settings = mSettings.mSettings[CD3DSettings::WINDOWED];
	if( bestDeviceCombo == NULL ) {
		settings.adapterInfo = NULL;
		settings.deviceInfo = NULL;
		settings.deviceCombo = NULL;
		return false;
	}
	
	mSettings.mMode = CD3DSettings::WINDOWED;
	settings.adapterInfo = bestAdapterInfo;
	settings.deviceInfo = bestDeviceInfo;
	settings.deviceCombo = bestDeviceCombo;
	settings.displayMode = primaryDesktopDM;
	mSettings.mWindowedWidth = mWindowClient.right - mWindowClient.left;
	mSettings.mWindowedHeight = mWindowClient.bottom - mWindowClient.top;
	if( mEnumeration.mUsesDepthBuffer )
		settings.depthStencilFormat = (D3DFORMAT)bestDeviceCombo->depthStencilFormats[0];
	settings.multisampleType = (D3DMULTISAMPLE_TYPE)bestDeviceCombo->multiSampleTypes[0];
	settings.multisampleQuality = 0;
	settings.vertexProcessing = bestDeviceInfo->vertexProcessings[0];
	settings.vsync = bestDeviceCombo->hasNoVSync ? false : true;
	return true;
}


/**
 *  Sets up mSettings with best available fullscreen mode, subject to the
 *  requireHAL and requireREF constraints. Returns false if no such
 *  mode can be found.
 */
bool CD3DApplication::findBestFullscreenMode( bool requireHAL, bool requireREF )
{
	// For fullscreen, default to first HAL DeviceCombo that supports the
	// current desktop display mode, or any display mode if HAL is not
	// compatible with the desktop mode, or non-HAL if no HAL is available
	D3DDISPLAYMODE adapterDesktopDM;
	D3DDISPLAYMODE bestAdapterDesktopDM;
	D3DDISPLAYMODE bestDM;
	bestAdapterDesktopDM.Width = 0;
	bestAdapterDesktopDM.Height = 0;
	bestAdapterDesktopDM.Format = D3DFMT_UNKNOWN;
	bestAdapterDesktopDM.RefreshRate = 0;
	
	const SD3DAdapterInfo* bestAdapterInfo = NULL;
	const SD3DDeviceInfo* bestDeviceInfo = NULL;
	const SD3DDeviceCombo* bestDeviceCombo = NULL;
	
	for( int iai = 0; iai < mEnumeration.mAdapterInfos.size(); ++iai ) {
		const SD3DAdapterInfo* adInfo = mEnumeration.mAdapterInfos[iai];
		mD3D->GetAdapterDisplayMode( adInfo->adapterOrdinal, &adapterDesktopDM );
		for( int idi = 0; idi < adInfo->deviceInfos.size(); ++idi ) {
			const SD3DDeviceInfo* devInfo = adInfo->deviceInfos[idi];
			if( requireHAL && devInfo->caps.getDeviceType() != D3DDEVTYPE_HAL )
				continue;
			if( requireREF && devInfo->caps.getDeviceType() != D3DDEVTYPE_REF )
				continue;
			for( int idc = 0; idc < devInfo->deviceCombos.size(); ++idc ) {
				const SD3DDeviceCombo* devCombo = devInfo->deviceCombos[idc];
				bool matchesBB = (devCombo->backBufferFormat == devCombo->adapterFormat);
				bool matchesDesktop = (devCombo->adapterFormat == adapterDesktopDM.Format);
				if( devCombo->isWindowed )
					continue;
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match
				// better), save it
				if( !bestDeviceCombo ||
					bestDeviceCombo->deviceType != D3DDEVTYPE_HAL && devInfo->caps.getDeviceType() == D3DDEVTYPE_HAL ||
					devCombo->deviceType == D3DDEVTYPE_HAL && bestDeviceCombo->adapterFormat != adapterDesktopDM.Format && matchesDesktop ||
					devCombo->deviceType == D3DDEVTYPE_HAL && matchesDesktop && matchesBB )
				{
					bestAdapterDesktopDM = adapterDesktopDM;
					bestAdapterInfo = adInfo;
					bestDeviceInfo = devInfo;
					bestDeviceCombo = devCombo;
					if( devInfo->caps.getDeviceType() == D3DDEVTYPE_HAL && matchesDesktop && matchesBB )
					{
						// This fullscreen device combo looks great -- take it
						goto _endComboSearch;
					}
					// Otherwise keep looking for a better combo
				}
			}
		}
	}
_endComboSearch:
	CD3DSettings::SSettings& settings = mSettings.mSettings[CD3DSettings::FULLSCREEN];
	if( bestDeviceCombo == NULL ) {
		settings.adapterInfo = NULL;
		settings.deviceInfo = NULL;
		settings.deviceCombo = NULL;
		return false;
	}
	
	// Need to find a display mode on the best adapter that uses
	// bestDeviceCombo->adapterFormat and is as close to bestAdapterDesktopDM's
	// res as possible
	bestDM.Width = 0;
	bestDM.Height = 0;
	bestDM.Format = D3DFMT_UNKNOWN;
	bestDM.RefreshRate = 0;
	for( int idm = 0; idm < bestAdapterInfo->displayModes.size(); ++idm ) {
		const D3DDISPLAYMODE& pdm = bestAdapterInfo->displayModes[idm];
		if( pdm.Format != bestDeviceCombo->adapterFormat )
			continue;
		if( pdm.Width == bestAdapterDesktopDM.Width &&
			pdm.Height == bestAdapterDesktopDM.Height && 
			pdm.RefreshRate == bestAdapterDesktopDM.RefreshRate )
		{
			// found a perfect match, so stop
			bestDM = pdm;
			break;
		} else if( pdm.Width == bestAdapterDesktopDM.Width &&
			pdm.Height == bestAdapterDesktopDM.Height && 
			pdm.RefreshRate > bestDM.RefreshRate )
		{
			// refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			bestDM = pdm;
		} else if( pdm.Width == bestAdapterDesktopDM.Width ) {
			// width matches, so keep this and keep looking
			bestDM = pdm;
		} else if( bestDM.Width == 0 ) {
			// we don't have anything better yet, so keep this and keep looking
			bestDM = pdm;
		}
	}
	
	mSettings.mMode = CD3DSettings::FULLSCREEN;

	settings.adapterInfo = bestAdapterInfo;
	settings.deviceInfo = bestDeviceInfo;
	settings.deviceCombo = bestDeviceCombo;
	settings.displayMode = bestDM;
	if( mEnumeration.mUsesDepthBuffer )
		settings.depthStencilFormat = (D3DFORMAT)bestDeviceCombo->depthStencilFormats[0];
	settings.multisampleType = (D3DMULTISAMPLE_TYPE)bestDeviceCombo->multiSampleTypes[0];
	settings.multisampleQuality = 0;
	settings.vertexProcessing = bestDeviceInfo->vertexProcessings[0];
	settings.vsync = (mVSyncFullscreen && bestDeviceCombo->hasVSync) ? true : false;

	return true;
}


HRESULT CD3DApplication::chooseInitialD3DSettings()
{
	bool foundFullscreen = findBestFullscreenMode( true, false );
	bool foundWindowed = findBestWindowedMode( true, false );
	
	if( (mStartFullscreen || !foundWindowed || mSSMode==SM_FULL) && foundFullscreen )
		mSettings.mMode = CD3DSettings::FULLSCREEN;

	if( !foundFullscreen && (!foundWindowed || mSSMode==SM_FULL) )
		return (HRESULT)NOCOMPATIBLEDEVICES;

	modifyD3DSettingsFromPref();
	
	return S_OK;
}

void CD3DApplication::modifyD3DSettingsFromPref()
{
	int i;

	const SD3DSettingsPref& p = mSettingsPref;
	CD3DSettings s = mSettings;
	const CD3DEnumeration& en = mEnumeration;

	// windowed/fullscreen
	if( p.mode != -1 ) {
		s.mMode = (CD3DSettings::eMode)p.mode;
		if( !s.mSettings[s.mMode].adapterInfo ||
			!s.mSettings[s.mMode].deviceInfo ||
			!s.mSettings[s.mMode].deviceCombo )
			return;
	} else {
		return;
	}

	bool mismatch;
	
	// adapter
	const SD3DAdapterInfo* adapter = 0;
	mismatch = true;
	for( i = 0; i < en.mAdapterInfos.size(); ++i ) {
		adapter = mEnumeration.mAdapterInfos[i];
		if( adapter->adapterID.VendorId == p.adapterVendorID && adapter->adapterID.DeviceId == p.adapterDeviceID ) {
			mismatch = false;
			s.mSettings[s.mMode].adapterInfo = adapter;
			break;
		}
	}
	if( mismatch )
		return;

	// device
	const SD3DDeviceInfo* device = 0;
	mismatch = true;
	for( i = 0; i < adapter->deviceInfos.size(); ++i ) {
		device = adapter->deviceInfos[i];
		if( device->caps.getDeviceType() == p.device ) {
			mismatch = false;
			s.mSettings[s.mMode].deviceInfo = device;
			break;
		}
	}
	if( mismatch )
		return;

	// find vertex processing
	mismatch = true;
	for( i = 0; i < device->vertexProcessings.size(); ++i ) {
		if( device->vertexProcessings[i] == p.vertexProc ) {
			mismatch = false;
			s.mSettings[s.mMode].vertexProcessing = (CD3DDeviceCaps::eVertexProcessing)p.vertexProc;
			break;
		}
	}
	if( mismatch )
		return;

	// combo
	const SD3DDeviceCombo* devCombo = 0;
	if( s.mMode == CD3DSettings::WINDOWED ) {
		// windowed mode: window size, refresh rate and format aren't stored in pref
		s.mSettings[s.mMode].deviceCombo = mSettings.mSettings[s.mMode].deviceCombo;
		s.mSettings[s.mMode].displayMode = mSettings.mSettings[s.mMode].displayMode;
		devCombo = s.mSettings[s.mMode].deviceCombo;
	} else {
		// fullscreen mode
		mismatch = true;
		for( i = 0; i < device->deviceCombos.size(); ++i ) {
			devCombo = device->deviceCombos[i];
			if( !devCombo->isWindowed && devCombo->deviceType == p.device &&
				devCombo->adapterFormat == p.fsFormat && devCombo->backBufferFormat == p.backbuffer )
			{
				mismatch = false;
				s.mSettings[s.mMode].deviceCombo = devCombo;
				break;
			}
		}
		if( mismatch )
			return;

		// find display mode
		const D3DDISPLAYMODE* dm = 0;
		mismatch = true;
		for( i = 0; i < adapter->displayModes.size(); ++i ) {
			dm = &adapter->displayModes[i];
			if( dm->Format != devCombo->adapterFormat )
				continue;
			if( dm->Width == p.fsWidth && dm->Height == p.fsHeight && dm->RefreshRate == p.fsRefresh ) {
				mismatch = false;
				s.mSettings[s.mMode].displayMode = *dm;
				break;
			}
		}
		if( mismatch )
			return;
	}

	// find AA
	mismatch = true;
	for( i = 0; i < devCombo->multiSampleTypes.size(); ++i ) {
		if( devCombo->multiSampleTypes[i] == p.fsaaType ) {
			mismatch = false;
			s.mSettings[s.mMode].multisampleType = (D3DMULTISAMPLE_TYPE)p.fsaaType;
			break;
		}
	}
	if( mismatch )
		return;

	if( devCombo->multiSampleQualities[p.fsaaType] > p.fsaaQuality ) {
		s.mSettings[s.mMode].multisampleQuality = p.fsaaQuality;
	} else
		return;

	// find z/stencil
	mismatch = true;
	for( i = 0; i < devCombo->depthStencilFormats.size(); ++i ) {
		if( devCombo->depthStencilFormats[i] == p.zstencil ) {
			mismatch = false;
			s.mSettings[s.mMode].depthStencilFormat = (D3DFORMAT)p.zstencil;
			break;
		}
	}
	if( mismatch )
		return;

	// vsync
	if( p.vsync != -1 ) {
		s.mSettings[s.mMode].vsync = p.vsync ? true : false;
	} else {
		return;
	}

	// finally all ok, apply the settings
	mSettings = s;
}


void CD3DApplication::ssReadSettings()
{
	// TBD
	/*
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
	*/
}

//  Write the registry settings that affect how the screens are set up and used.
void CD3DApplication::ssWriteSettings()
{
	// TBD
	/*
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
	*/
}

eSaverMode CD3DApplication::ssParseCmdLine(	TCHAR* cmdLine )
{
	mSSHwndParent = NULL;
	
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
			convertGenericStringToAnsiCb( strCommandLine, cmdLine, sizeof(strCommandLine) );
			mSSHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mSSHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
#endif
		} else {
			mSSHwndParent = NULL;
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
			convertGenericStringToAnsiCb(strCommandLine, cmdLine, sizeof(strCommandLine));
			mSSHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mSSHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
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
			convertGenericStringToAnsiCb(strCommandLine, cmdLine, sizeof(strCommandLine));
			mSSHwndParent = (HWND)(_atoi64(strCommandLine));
#else
			mSSHwndParent = (HWND)LongToHandle(_ttol(cmdLine));
#endif
		}
		return SM_PASSCHANGE;
		
	default:
		// All other options => run the screensaver (typically this is "/s")
		return SM_FULL;
	}
}

// --------------------------------------------------------------------------

/** Message handling function. */
LRESULT CD3DApplication::msgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_USER:
		// Screensaver:
		if( mSSMode != SM_NONE ) {
			// All initialization messages have gone through.  Allow
			// 500ms of idle time, then proceed with initialization.
			SetTimer( hWnd, 1, 500, NULL );
		}
		break;
	case WM_TIMER:
		// Screensaver:
		if( mSSMode != SM_NONE ) {
			// Initial idle time is done, proceed with initialization.
			mSSInStartingPause = false;
			KillTimer( hWnd, 1 );
		}
		break;

	case WM_PAINT:
		// TBD: saver?
		
		// handle paint messages when the app is paused
		if( mD3DDevice && !mActive && mWindowed && mDeviceObjectsInited && mDeviceObjectsRestored ) {
			performOneTime();
			mD3DDevice->Present( NULL, NULL, NULL, NULL );
		}
		break;
		
	case WM_ERASEBKGND:
		// Erase background if checking password
		if( mSSMode != SM_NONE && !mSSCheckingPassword )
			return true; // don't erase this window
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;
		
	case WM_ENTERSIZEMOVE:
		// halt movement while the app is sizing or moving
		pause( true );
		break;
		
	case WM_SIZE:
		// pick up possible changes to window style due to maximize, etc.
		if( mWindowed && mHWnd != NULL )
			mWindowStyle = GetWindowLong( mHWnd, GWL_STYLE );
		
		if( SIZE_MINIMIZED == wParam ) {
			if( mClipCursorWhenFullscreen && !mWindowed )
				ClipCursor( NULL );
			pause( true ); // pause while we're minimized
			mMinimized = true;
			mMaximized = false;
		} else if( SIZE_MAXIMIZED == wParam ) {
			if( mMinimized )
				pause( false ); // unpause since we're no longer minimized
			mMinimized = false;
			mMaximized = true;
			handlePossibleSizeChange();
		} else if( SIZE_RESTORED == wParam ) {
			if( mMaximized ) {
				mMaximized = false;
				handlePossibleSizeChange();
			} else if( mMinimized ) {
				pause( false ); // unpause since we're no longer minimized
				mMinimized = false;
				handlePossibleSizeChange();
			} else {
				// If we're neither maximized nor minimized, the window size 
				// is changing by the user dragging the window edges.  In this 
				// case, we don't reset the device yet -- we wait until the 
				// user stops dragging, and a WM_EXITSIZEMOVE message comes.
			}
		}
		break;
		
	case WM_EXITSIZEMOVE:
		pause( false );
		handlePossibleSizeChange();
		break;
		
	case WM_SETCURSOR:
		// screensaver
		if( mSSMode	!= SM_NONE ) {
			if( mSSMode == SM_FULL && !mSSCheckingPassword ) {
				// Hide cursor
				SetCursor( NULL );
				return true;
			}
		} else {
			// turn off Windows cursor in fullscreen mode
			if( mActive && !mWindowed ) {
				SetCursor( NULL );
				if( mShowCursorWhenFullscreen )
					mD3DDevice->ShowCursor( true );
				return true;
			}
		}
		break;
		
	case WM_MOUSEMOVE:
		if( mSSMode != SM_NONE ) {
			if( mSSMode != SM_TEST ) {
				static INT xPrev = -1;
				static INT yPrev = -1;
				INT xCur = GET_X_LPARAM(lParam);
				INT yCur = GET_Y_LPARAM(lParam);
				if( xCur != xPrev || yCur != yPrev )
				{
					xPrev = xCur;
					yPrev = yCur;
					mSSMouseMoveCount++;
					if( mSSMouseMoveCount > 5 )
						ssInterrupt();
				}
			}
		} else {
			if( mActive && mD3DDevice != NULL ) {
				POINT ptCursor;
				GetCursorPos( &ptCursor );
				if( !mWindowed )
					ScreenToClient( mHWnd, &ptCursor );
				mD3DDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
			}
		}
		break;
		
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if( mSSMode != SM_NONE && mSSMode != SM_TEST )
			ssInterrupt();
		break;

	case WM_ACTIVATEAPP:
		if( wParam == FALSE && mSSMode != SM_NONE && mSSMode != SM_TEST )
			ssInterrupt();
		break;

	case WM_ENTERMENULOOP:
		// pause the app when menus are displayed
		pause( true );
		break;
		
	case WM_EXITMENULOOP:
		pause( false );
		break;
		
	case WM_NCHITTEST:
		// prevent the user from selecting the menu in fullscreen mode
		if( !mWindowed )
			return HTCLIENT;
		break;
		
	case WM_POWERBROADCAST:
		if( mSSMode != SM_NONE ) {
			if( wParam == PBT_APMSUSPEND && mSSVerifyPasswordProc == NULL )
				ssInterrupt();
		} else {
			switch( wParam ) {
#ifndef PBT_APMQUERYSUSPEND
#define PBT_APMQUERYSUSPEND 0x0000
#endif
			case PBT_APMQUERYSUSPEND:
				// At this point, the app should save any data for open
				// network connections, files, etc., and prepare to go into
				// a suspended mode.
				return true;
			
#ifndef PBT_APMRESUMESUSPEND
#define PBT_APMRESUMESUSPEND 0x0007
#endif
			case PBT_APMRESUMESUSPEND:
				// At this point, the app should recover any data, network
				// connections, files, etc., and resume running from when
				// the app was suspended.
				return true;
			}
		}
		break;
		
	case WM_SYSCOMMAND:
		switch( wParam ) {
		case SC_NEXTWINDOW:
		case SC_PREVWINDOW:
		case SC_SCREENSAVE:
		case SC_CLOSE:
			if( mSSMode == SM_FULL )
				return FALSE;
			break;
		// prevent moving/sizing and power loss in fullscreen mode
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if( false == mWindowed )
				return 1;
			break;
		}
		break;
		
	case WM_COMMAND:
		if( mSSMode == SM_NONE ) {
			switch( LOWORD(wParam) ) {
			case IDM_CHANGEDEVICE:
				// prompt the user to select a new device or mode
				pause( true );
				userSelectNewDevice();
				pause( false );
				return 0;
				
			case IDM_TOGGLEFULLSCREEN:
				// toggle the fullscreen/window mode
				pause( true );
				if( FAILED( toggleFullscreen() ) )
					displayErrorMsg( (HRESULT)RESETFAILED, APPMUSTEXIT );
				pause( false ); 					   
				return 0;
				
			case IDM_EXIT:
				// recieved key/menu command to exit app
				SendMessage( hWnd, WM_CLOSE, 0, 0 );
				return 0;
			}
		}
		break;
		
	case WM_CLOSE:
		{
			static boolean closing = false;
			if( !closing ) {
				closing = true;
				close();
				HMENU hMenu = GetMenu(hWnd);
				if( hMenu != NULL )
					DestroyMenu( hMenu );
				DestroyWindow( hWnd );
				PostQuitMessage(0);
				mHWnd = NULL;
			}
		}
		return 0;
	case WM_DESTROY:
		// TBD: need something?
		//shutdownSaver();
		break;
	}
	
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//  A message was received (mouse move, keydown, etc.) that may mean
//	the screen saver should show the password dialog and/or shut down.
void CD3DApplication::ssInterrupt()
{
	HRESULT hr;
	bool passwordOK = false;
	if( mSSMode == SM_TEST || mSSMode == SM_FULL && !mSSCheckingPassword ) {
		if( mSSIsWin9x && mSSMode == SM_FULL ) {
			// If no VerifyPassword function, then no password is set 
			// or we're not on 9x. 
			if( mSSVerifyPasswordProc != NULL ) {
				// Shut down D3D device so we can show a Windows dialog
				cleanup3DEnvironment();
				
				// Make sure window covers the whole screen,
				// even after deleting D3D device (which may have caused
				// mode changes)
				ShowWindow( mHWnd, SW_RESTORE );
				ShowWindow( mHWnd, SW_MAXIMIZE );
				
				mSSCheckingPassword = true;
				passwordOK = mSSVerifyPasswordProc( mHWnd ) ? true : false;
				mSSCheckingPassword = false;
				
				if( passwordOK ) {
					// all ok...
				} else {
					// Back to screen saving...
					SetCursor( NULL );
					mSSMouseMoveCount = 0;
					
					// Recreate D3D device
					hr = initialize3DEnvironment();
					return;
				}
			}
		}
		doClose();
	}
}

void CD3DApplication::close()
{
	// Unflag screensaver running if in full on mode
	if( mSSMode == SM_FULL ) {
		BOOL bUnused;
		SystemParametersInfo( SPI_SCREENSAVERRUNNING, FALSE, &bUnused, 0 );
	}

	cleanup3DEnvironment();
	safeRelease( mD3D );
	shutdown();

	// Unload the password DLL (if we loaded it)
	if( mSSPasswordDLL != NULL ) {
		FreeLibrary( mSSPasswordDLL );
		mSSPasswordDLL = NULL;
	}
}

void CD3DApplication::doClose()
{
	PostMessage( mHWnd, WM_CLOSE, 0, 0 );
}

/** Reset the device if the client area size has changed. */
HRESULT CD3DApplication::handlePossibleSizeChange()
{
	HRESULT hr = S_OK;
	RECT clientOld;
	clientOld = mWindowClient;
	
	if( mIgnoreSizeChange )
		return S_OK;
	
	// update window properties
	GetWindowRect( mHWnd, &mWindowBounds );
	GetClientRect( mHWnd, &mWindowClient );
	
	if( clientOld.right - clientOld.left != mWindowClient.right - mWindowClient.left ||
		clientOld.bottom - clientOld.top != mWindowClient.bottom - mWindowClient.top )
	{
		// A new window size will require a new backbuffer
		// size, so the 3D structures must be changed accordingly.
		pause( true );
		
		mPresentParams.BackBufferWidth  = mWindowClient.right - mWindowClient.left;
		mPresentParams.BackBufferHeight = mWindowClient.bottom - mWindowClient.top;
		
		if( mD3DDevice != NULL ) {
			// Reset the 3D environment
			if( FAILED( hr = reset3DEnvironment() ) ) {
				if( hr != D3DERR_OUTOFVIDEOMEMORY )
					hr = (HRESULT)RESETFAILED;
				displayErrorMsg( hr, APPMUSTEXIT );
			}
		}
		pause( false );
	}
	return hr;
}



HRESULT CD3DApplication::initialize3DEnvironment()
{
	HRESULT hr;

	mSSMouseMoveCount = 0;
	
	const SD3DAdapterInfo& adInfo = mSettings.getAdapterInfo();
	const SD3DDeviceInfo& devInfo = mSettings.getDeviceInfo();
	
	mWindowed = (mSettings.mMode == CD3DSettings::WINDOWED);
	
	// Prepare window for possible windowed/fullscreen change
	adjustWindowForChange();
	
	// Set up the presentation parameters
	bool okparams = buildPresentParamsFromSettings();
	if( !okparams )
		return CANTTOGGLEFULLSCREEN;
	
	if( devInfo.caps.getCaps().PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE ) {
		// Warn user about null ref device that can't render anything
		displayErrorMsg( (HRESULT)NULLREFDEVICE, NONE );
	}
	
	DWORD behaviorFlags = 0;
	switch( mSettings.getVertexProcessing() ) {
	case CD3DDeviceCaps::VP_PURE_HW:
		behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		break;
	case CD3DDeviceCaps::VP_HW:
		behaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		break;
	case CD3DDeviceCaps::VP_MIXED:
		behaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
		break;
	case CD3DDeviceCaps::VP_SW:
		behaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		break;
	default:
		assert( false );
	}
	
	// Araz:
	behaviorFlags |= D3DCREATE_FPU_PRESERVE;
	behaviorFlags |= D3DCREATE_MULTITHREADED;

	// create the device
	hr = mD3D->CreateDevice(
		mSettings.getAdapterOrdinal(), devInfo.caps.getDeviceType(),
		mHWndFocus, behaviorFlags, &mPresentParams,
		&mD3DDevice );
	
	if( SUCCEEDED(hr) ) {
		// When moving from fullscreen to windowed mode, it is important to
		// adjust the window size after recreating the device rather than
		// beforehand to ensure that you get the window size you want.	For
		// example, when switching from 640x480 fullscreen to windowed with
		// a 1000x600 window on a 1024x768 desktop, it is impossible to set
		// the window size to 1000x600 until after the display mode has
		// changed to 1024x768, because windows cannot be larger than the
		// desktop.
		if( mWindowed ) {
			SetWindowPos( mHWnd, HWND_NOTOPMOST,
				mWindowBounds.left, mWindowBounds.top,
				( mWindowBounds.right - mWindowBounds.left ),
				( mWindowBounds.bottom - mWindowBounds.top ),
				SWP_SHOWWINDOW );
		}
		
		// Store device Caps
		mD3DDevice->GetDeviceCaps( &mD3DCaps );
		mCreateFlags = behaviorFlags;
		
		// Store device description
		if( devInfo.caps.getDeviceType() == D3DDEVTYPE_REF )
			lstrcpy( mDeviceStats, TEXT("REF") );
		else if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL )
			lstrcpy( mDeviceStats, TEXT("HAL") );
		else if( devInfo.caps.getDeviceType() == D3DDEVTYPE_SW )
			lstrcpy( mDeviceStats, TEXT("SW") );
		
		if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING && behaviorFlags & D3DCREATE_PUREDEVICE ) {
			if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL )
				lstrcat( mDeviceStats, TEXT(" (pure hw vp)") );
			else
				lstrcat( mDeviceStats, TEXT(" (simulated pure hw vp)") );
		} else if( behaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING ) {
			if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL )
				lstrcat( mDeviceStats, TEXT(" (hw vp)") );
			else
				lstrcat( mDeviceStats, TEXT(" (simulated hw vp)") );
		} else if( behaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING ) {
			if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL )
				lstrcat( mDeviceStats, TEXT(" (mixed vp)") );
			else
				lstrcat( mDeviceStats, TEXT(" (simulated mixed vp)") );
		} else if( behaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING ) {
			lstrcat( mDeviceStats, TEXT(" (sw vp)") );
		}
		
		if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL ) {
			// Be sure not to overflow mDeviceStats when appending the adapter 
			// description, since it can be long.  Note that the adapter description
			// is initially CHAR and must be converted to TCHAR.
			lstrcat( mDeviceStats, TEXT(": ") );
			const int cchDesc = sizeof(adInfo.adapterID.Description);
			TCHAR szDescription[cchDesc];
			dingus::convertAnsiStringToGenericCch( szDescription, 
				adInfo.adapterID.Description, cchDesc );
			int maxAppend = sizeof(mDeviceStats) / sizeof(TCHAR) -
				lstrlen( mDeviceStats ) - 1;
			_tcsncat( mDeviceStats, szDescription, maxAppend );
		}
		
		// Store render target surface desc
		IDirect3DSurface9* backBuffer = NULL;
		mD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer );
		backBuffer->GetDesc( &mBackBuffer );
		backBuffer->Release();
		
		// Set up the fullscreen cursor
		if( mShowCursorWhenFullscreen && !mWindowed ) {
			HCURSOR hCursor;
#ifdef _WIN64
			hCursor = (HCURSOR)GetClassLongPtr( mHWnd, GCLP_HCURSOR );
#else
			hCursor = (HCURSOR)ULongToHandle( GetClassLong( mHWnd, GCL_HCURSOR ) );
#endif
			dingus::setDeviceCursor( *mD3DDevice, hCursor );
			mD3DDevice->ShowCursor( true );
		}
		
		// Confine cursor to fullscreen window
		if( mClipCursorWhenFullscreen ) {
			if( !mWindowed ) {
				RECT rcWindow;
				GetWindowRect( mHWnd, &rcWindow );
				ClipCursor( &rcWindow );
			} else {
				ClipCursor( NULL );
			}
		}
		
		mSSMouseMoveCount = 0; // make sure all changes don't count as mouse moves

		// Initialize the app's device-dependent objects
		hr = internalCreateDevice();
		if( FAILED(hr) ) {
			internalDeleteDevice();
		} else {
			mDeviceObjectsInited = true;
			hr = internalActivateDevice();
			if( FAILED(hr) ) {
				internalPassivateDevice();
			} else {
				mDeviceObjectsRestored = true;
				return S_OK;
			}
		}
		
		// Cleanup before we try again
		cleanup3DEnvironment();
	}
	
	// If that failed, fall back to the reference rasterizer
	if( devInfo.caps.getDeviceType() == D3DDEVTYPE_HAL ) {
		if( findBestWindowedMode( false, true ) ) {
			mWindowed = true;
			adjustWindowForChange();
			// Make sure main window isn't topmost, so error message is visible
			SetWindowPos( mHWnd, HWND_NOTOPMOST,
				mWindowBounds.left, mWindowBounds.top,
				( mWindowBounds.right - mWindowBounds.left ),
				( mWindowBounds.bottom - mWindowBounds.top ),
				SWP_SHOWWINDOW );
			
			// Let the user know we are switching from HAL to the reference rasterizer
			displayErrorMsg( hr, SWITCHEDTOREF );
			
			hr = initialize3DEnvironment();
			
			mSSMouseMoveCount = 0; // make sure all changes don't count as mouse moves
		}
	}
	return hr;
}


bool CD3DApplication::buildPresentParamsFromSettings()
{
	if( &mSettings.getDeviceCombo() == NULL )
		return false;

	mPresentParams.Windowed				= (mSettings.mMode == CD3DSettings::WINDOWED);
	mPresentParams.BackBufferCount		= 1;
	mPresentParams.MultiSampleType		= mSettings.getMultiSampleType();
	mPresentParams.MultiSampleQuality	= mSettings.getMultiSampleQuality();
	mPresentParams.SwapEffect			= D3DSWAPEFFECT_DISCARD;
	mPresentParams.EnableAutoDepthStencil = mEnumeration.mUsesDepthBuffer;
	mPresentParams.hDeviceWindow		= mHWnd;
	if( mEnumeration.mUsesDepthBuffer ) {
		//mPresentParams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		mPresentParams.Flags = 0;
		mPresentParams.AutoDepthStencilFormat = mSettings.getDepthStencilFormat();
	} else {
		mPresentParams.Flags = 0;
	}
	
	if( mWindowed ) {
		mPresentParams.BackBufferWidth  = mWindowClient.right - mWindowClient.left;
		mPresentParams.BackBufferHeight = mWindowClient.bottom - mWindowClient.top;
		mPresentParams.BackBufferFormat = mSettings.getDeviceCombo().backBufferFormat;
		mPresentParams.FullScreen_RefreshRateInHz = 0;
		mPresentParams.PresentationInterval = mSettings.isVSync() ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	} else {
		mPresentParams.BackBufferWidth  = mSettings.getDisplayMode().Width;
		mPresentParams.BackBufferHeight = mSettings.getDisplayMode().Height;
		mPresentParams.BackBufferFormat = mSettings.getDeviceCombo().backBufferFormat;
		mPresentParams.FullScreen_RefreshRateInHz = mSettings.getDisplayMode().RefreshRate;
		mPresentParams.PresentationInterval = mSettings.isVSync() ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	return true;
}


HRESULT CD3DApplication::reset3DEnvironment()
{
	HRESULT hr;
	
	// Release all vidmem objects
	if( mDeviceObjectsRestored ) {
		mDeviceObjectsRestored = false;
		internalPassivateDevice();
	}
	// Reset the device
	if( FAILED( hr = mD3DDevice->Reset( &mPresentParams ) ) )
		return hr;
	
	// Store render target surface desc
	IDirect3DSurface9* backBuffer;
	mD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer );
	backBuffer->GetDesc( &mBackBuffer );
	backBuffer->Release();
	
	// Set up the fullscreen cursor
	if( mShowCursorWhenFullscreen && !mWindowed ) {
		HCURSOR hCursor;
#ifdef _WIN64
		hCursor = (HCURSOR)GetClassLongPtr( mHWnd, GCLP_HCURSOR );
#else
		hCursor = (HCURSOR)ULongToHandle( GetClassLong( mHWnd, GCL_HCURSOR ) );
#endif
		dingus::setDeviceCursor( *mD3DDevice, hCursor );
		mD3DDevice->ShowCursor( true );
	}
	
	// Confine cursor to fullscreen window
	if( mClipCursorWhenFullscreen ) {
		if( !mWindowed ) {
			RECT rcWindow;
			GetWindowRect( mHWnd, &rcWindow );
			ClipCursor( &rcWindow );
		} else {
			ClipCursor( NULL );
		}
	}
	
	// Initialize the app's device-dependent objects
	hr = internalActivateDevice();
	if( FAILED(hr) ) {
		internalPassivateDevice();
		return hr;
	}
	mDeviceObjectsRestored = true;
	
	return S_OK;
}


HRESULT CD3DApplication::internalCreateDevice()
{
	assert( mD3DDevice );
	CD3DDevice& dev = CD3DDevice::getInstance();
	dev.setDevice( mD3DDevice, mSettings.getVertexProcessing() );
	
	HRESULT hr = createDeviceObjects();
	return hr;
}

HRESULT CD3DApplication::internalDeleteDevice()
{
	HRESULT hr = deleteDeviceObjects();
	CD3DDevice::getInstance().setDevice( NULL, CD3DDeviceCaps::VP_SW );
	return hr;
}

HRESULT CD3DApplication::internalActivateDevice()
{
	CD3DDevice::getInstance().activateDevice( mHWnd );
	HRESULT hr = activateDeviceObjects();
	return hr;
}

HRESULT CD3DApplication::internalPassivateDevice()
{
	HRESULT hr = passivateDeviceObjects();
	CD3DDevice::getInstance().passivateDevice();
	return hr;
}





/** Called when user toggles between fullscreen mode and windowed mode. */
HRESULT CD3DApplication::toggleFullscreen()
{
	HRESULT hr;
	int adOrdinalOld = mSettings.getAdapterOrdinal();
	D3DDEVTYPE devTypeOld = mSettings.getDevType();
	
	pause( true );
	mIgnoreSizeChange = true;
	
	// Toggle the windowed state
	mWindowed = !mWindowed;
	mSettings.mMode = (mSettings.mMode==CD3DSettings::WINDOWED) ? (CD3DSettings::FULLSCREEN) : (CD3DSettings::WINDOWED);
	
	// Prepare window for windowed/fullscreen change
	adjustWindowForChange();
	
	// If adapterOrdinal and deviceType are the same, we can just do a Reset().
	// If they've changed, we need to do a complete device teardown/rebuild.
	if( mSettings.getAdapterOrdinal() == adOrdinalOld && mSettings.getDevType() == devTypeOld ) {
		// Reset the 3D device
		bool okparams = buildPresentParamsFromSettings();
		if( okparams )
			hr = reset3DEnvironment();
		else
			hr = CANTTOGGLEFULLSCREEN;
	} else {
		cleanup3DEnvironment();
		hr = initialize3DEnvironment();
	}
	
	if( FAILED( hr ) ) {
		if( hr != D3DERR_OUTOFVIDEOMEMORY )
			hr = RESETFAILED;
		mIgnoreSizeChange = false;
		if( !mWindowed ) {
			// Restore window type to windowed mode
			mWindowed = !mWindowed;
			mSettings.mMode = CD3DSettings::WINDOWED;
			adjustWindowForChange();
			SetWindowPos( mHWnd, HWND_NOTOPMOST,
				mWindowBounds.left, mWindowBounds.top,
				( mWindowBounds.right - mWindowBounds.left ),
				( mWindowBounds.bottom - mWindowBounds.top ),
				SWP_SHOWWINDOW );
		}
		return displayErrorMsg( hr, APPMUSTEXIT );
	}
	
	mIgnoreSizeChange = false;
	
	// When moving from fullscreen to windowed mode, it is important to
	// adjust the window size after resetting the device rather than
	// beforehand to ensure that you get the window size you want.	For
	// example, when switching from 640x480 fullscreen to windowed with
	// a 1000x600 window on a 1024x768 desktop, it is impossible to set
	// the window size to 1000x600 until after the display mode has
	// changed to 1024x768, because windows cannot be larger than the
	// desktop.
	if( mWindowed ) {
		SetWindowPos( mHWnd, HWND_NOTOPMOST,
			mWindowBounds.left, mWindowBounds.top,
			( mWindowBounds.right - mWindowBounds.left ),
			( mWindowBounds.bottom - mWindowBounds.top ),
			SWP_SHOWWINDOW );
	}
	
	pause( false );
	return S_OK;
}


/** Switch to a windowed mode, even if that means picking a new device and/or adapter. */
HRESULT CD3DApplication::forceWindowed()
{
	HRESULT hr;
	
	if( mWindowed )
		return S_OK;
	
	if( !findBestWindowedMode( false, false ) )
		return E_FAIL;

	mWindowed = true;
	
	// Now destroy the current 3D device objects, then reinitialize
	
	pause( true );
	
	// Release all scene objects that will be re-created for the new device
	cleanup3DEnvironment();
	
	// create the new device
	if( FAILED(hr = initialize3DEnvironment() ) )
		return displayErrorMsg( hr, APPMUSTEXIT );
	
	pause( false );
	return S_OK;
}


/**
 *  Prepare the window for a possible change between windowed mode and fullscreen
 *  mode. This function is virtual and thus can be overridden to provide
 *  different behavior, such as switching to an entirely different window for
 *  fullscreen mode (as in the MFC sample apps).
 */
HRESULT CD3DApplication::adjustWindowForChange()
{
	if( mWindowed ) {
		// Set windowed-mode style
		SetWindowLong( mHWnd, GWL_STYLE, mWindowStyle );
		if( mHMenu != NULL ) {
			SetMenu( mHWnd, mHMenu );
			mHMenu = NULL;
		}
	} else {
		// Set fullscreen-mode style
		SetWindowLong( mHWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
		if( mHMenu == NULL ) {
			mHMenu = GetMenu( mHWnd );
			SetMenu( mHWnd, NULL );
		}
	}
	return S_OK;
}


/**
 *  Displays a dialog so the user can select a new adapter, device, or
 *  display mode, and then recreates the 3D environment if needed
 */
HRESULT CD3DApplication::userSelectNewDevice()
{
	// Can't display dialogs in fullscreen mode
	if( mWindowed == false ) {
		if( FAILED( toggleFullscreen() ) ) {
			displayErrorMsg( RESETFAILED, APPMUSTEXIT );
			return E_FAIL;
		}
	}
	
	bool dlgok = justShowSettingsDialog();
	if( !dlgok )
		return S_OK;

	return applySettings();
}

HRESULT CD3DApplication::applySettings()
{
	HRESULT hr;
	
	// Release all scene objects that will be re-created for the new device
	cleanup3DEnvironment();
	
	// Inform the display class of the change. It will internally
	// re-create valid surfaces, a d3ddevice, etc.
	if( FAILED( hr = initialize3DEnvironment() ) ) {
		if( hr != D3DERR_OUTOFVIDEOMEMORY )
			hr = RESETFAILED;
		if( !mWindowed ) {
			// Restore window type to windowed mode
			mWindowed = !mWindowed;
			mSettings.mMode = mWindowed ? (CD3DSettings::WINDOWED) : (CD3DSettings::FULLSCREEN);
			adjustWindowForChange();
			SetWindowPos( mHWnd, HWND_NOTOPMOST,
				mWindowBounds.left, mWindowBounds.top,
				( mWindowBounds.right - mWindowBounds.left ),
				( mWindowBounds.bottom - mWindowBounds.top ),
				SWP_SHOWWINDOW );
		}
		return displayErrorMsg( hr, APPMUSTEXIT );
	}
	
	return S_OK;
}

bool CD3DApplication::justShowSettingsDialog()
{
	CD3DSettingsDialogWin32 settingsDialog( mEnumeration, mSettings );
	if( settingsDialog.showDialog( mHInstance, mHWnd, *this ) != IDOK )
		return false;
	settingsDialog.getFinalSettings( mSettings );
	mWindowed = (mSettings.mMode == CD3DSettings::WINDOWED);
	return true;
}


int CD3DApplication::run()
{
	// First manage screensaver running modes
	if( mSSMode == SM_CONFIG || mSSMode == SM_PASSCHANGE )
		return 0; // done already

	//
	// some stuff for screensavers

	// Figure out if we're on Win9x
	OSVERSIONINFO osvi; 
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx( &osvi );
	mSSIsWin9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

	// If we're in full mode, and on 9x, then need to load the password DLL
	if( mSSMode == SM_FULL && mSSIsWin9x ) {
		// Only do this if the password is set - check registry:
		HKEY hKey; 
		if( RegCreateKeyEx( HKEY_CURRENT_USER, REGSTR_PATH_SCREENSAVE, 0, NULL, 0, KEY_READ, NULL, &hKey, NULL ) == ERROR_SUCCESS ) { 
			DWORD dwVal;
			DWORD dwSize = sizeof(dwVal); 
			if ( (RegQueryValueEx( hKey, REGSTR_VALUE_USESCRPASSWORD, NULL, NULL,
				(BYTE*)&dwVal, &dwSize ) == ERROR_SUCCESS) && dwVal ) 
			{ 
				mSSPasswordDLL = LoadLibrary( TEXT("PASSWORD.CPL") );
				if( mSSPasswordDLL )
					mSSVerifyPasswordProc = (SSVERIFYPASSWORDPROC)GetProcAddress( mSSPasswordDLL, "VerifyScreenSavePwd" );
				RegCloseKey( hKey );
			}
		}
	}

	// Flag as screensaver running if in full on mode
	if( mSSMode == SM_FULL ) {
		bool bUnused;
		SystemParametersInfo( SPI_SCREENSAVERRUNNING, TRUE, &bUnused, 0 );
	}
	

	//
	// message pump

	// Load keyboard accelerators
	HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );
	
	// Now we're ready to recieve and process Windows messages.
	bool gotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );
	
	while( WM_QUIT != msg.message ) {
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		if( mActive )
			gotMsg = ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );
		else
			gotMsg = ( GetMessage( &msg, NULL, 0U, 0U ) != 0 );
		
		if( gotMsg ) {
			// Translate and dispatch the message
			if( hAccel == NULL || mHWnd == NULL ||  0 == TranslateAccelerator( mHWnd, hAccel, &msg ) ) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		} else {
			if( mDeviceLost ) {
				// Yield some CPU time to other processes
				Sleep( 100 ); // 100 milliseconds
			}
			// render a frame during idle time (no messages are waiting)
			if( mActive ) {
				// some sleep if in screensaver mode
				if( mSSMode == SM_FULL || mSSMode == SM_PREVIEW )
					Sleep( 2 );
				if( FAILED( render3DEnvironment() ) )
					SendMessage( mHWnd, WM_CLOSE, 0, 0 );
			}
		}
	}
	if( hAccel != NULL )
		DestroyAcceleratorTable( hAccel );
	
	return (INT)msg.wParam;
}


/** Draws the scene. */
HRESULT CD3DApplication::render3DEnvironment()
{
	HRESULT hr;

	if( mDeviceLost ) {
		// Test the cooperative level to see if it's okay to render
		if( FAILED( hr = mD3DDevice->TestCooperativeLevel() ) ) {
			// If the device was lost, do not render until we get it back
			if( D3DERR_DEVICELOST == hr )
				return S_OK;
			
			// Check if the device needs to be reset.
			if( D3DERR_DEVICENOTRESET == hr ) {
				// If we are windowed, read the desktop mode and use the same
				// format for the back buffer
				if( mWindowed ) {
					const SD3DAdapterInfo& adInfo = mSettings.getAdapterInfo();
					mD3D->GetAdapterDisplayMode( adInfo.adapterOrdinal, &mSettings.getDisplayMode() );
					mPresentParams.BackBufferFormat = mSettings.getDisplayMode().Format;
				}
				
				if( FAILED( hr = reset3DEnvironment() ) )
					return hr;
			}
			return hr;
		}
		mDeviceLost = false;
	}
	
	// Timer
	// For debugging - so we get reproducable behaviour
	if( mDebugTimer ) {
		CSystemTimer::getInstance().updateFixed( time_value( time_value::FREQ / 60 ) ); // emulate 60FPS
	} else {
		CSystemTimer::getInstance().update();
	}
	
	// perform
	if( FAILED( hr = performOneTime() ) )
		return hr;

	updateStats();
	
	// Show the frame on the primary surface.
	hr = mD3DDevice->Present( NULL, NULL, NULL, NULL );
	if( D3DERR_DEVICELOST == hr )
		mDeviceLost = true;
	
	return S_OK;
}



void CD3DApplication::updateStats()
{
	// Keep track of the frame count
	static double lastTime = 0.0f;
	static int frameCount  = 0;
	double time = CSystemTimer::getInstance().getTimeS();
	++frameCount;
	
	// Update the scene stats once per second
	if( time - lastTime > 1.0 ) {
		mFPS = (float)(frameCount / (time - lastTime));
		lastTime = time;
		frameCount = 0;
		
		TCHAR strFmt[100];
		D3DFORMAT fmtAdapter = mSettings.getDisplayMode().Format;
		if( fmtAdapter == mBackBuffer.Format ) {
			lstrcpyn( strFmt, dingus::convertD3DFormatToString( fmtAdapter ), 100 );
		} else {
			_sntprintf( strFmt, 100, TEXT("backbuf %s, adapter %s"), 
				dingus::convertD3DFormatToString( mBackBuffer.Format ), 
				dingus::convertD3DFormatToString( fmtAdapter ) );
		}
		strFmt[99] = TEXT('\0');
		
		TCHAR strDepthFmt[100];
		if( mEnumeration.mUsesDepthBuffer ) {
			_sntprintf( strDepthFmt, 100, TEXT(" (%s)"), 
				dingus::convertD3DFormatToString( mSettings.getDepthStencilFormat() ) );
			strDepthFmt[99] = TEXT('\0');
		} else {
			// No depth buffer
			strDepthFmt[0] = TEXT('\0');
		}
		
		TCHAR* pstrMultiSample;
		switch( mSettings.getMultiSampleType() ) {
		case D3DMULTISAMPLE_NONMASKABLE:  pstrMultiSample = TEXT(" (Nonmaskable Multisample)"); break;
		case D3DMULTISAMPLE_2_SAMPLES:	pstrMultiSample = TEXT(" (2x Multisample)"); break;
		case D3DMULTISAMPLE_3_SAMPLES:	pstrMultiSample = TEXT(" (3x Multisample)"); break;
		case D3DMULTISAMPLE_4_SAMPLES:	pstrMultiSample = TEXT(" (4x Multisample)"); break;
		case D3DMULTISAMPLE_5_SAMPLES:	pstrMultiSample = TEXT(" (5x Multisample)"); break;
		case D3DMULTISAMPLE_6_SAMPLES:	pstrMultiSample = TEXT(" (6x Multisample)"); break;
		case D3DMULTISAMPLE_7_SAMPLES:	pstrMultiSample = TEXT(" (7x Multisample)"); break;
		case D3DMULTISAMPLE_8_SAMPLES:	pstrMultiSample = TEXT(" (8x Multisample)"); break;
		case D3DMULTISAMPLE_9_SAMPLES:	pstrMultiSample = TEXT(" (9x Multisample)"); break;
		case D3DMULTISAMPLE_10_SAMPLES: pstrMultiSample = TEXT(" (10x Multisample)"); break;
		case D3DMULTISAMPLE_11_SAMPLES: pstrMultiSample = TEXT(" (11x Multisample)"); break;
		case D3DMULTISAMPLE_12_SAMPLES: pstrMultiSample = TEXT(" (12x Multisample)"); break;
		case D3DMULTISAMPLE_13_SAMPLES: pstrMultiSample = TEXT(" (13x Multisample)"); break;
		case D3DMULTISAMPLE_14_SAMPLES: pstrMultiSample = TEXT(" (14x Multisample)"); break;
		case D3DMULTISAMPLE_15_SAMPLES: pstrMultiSample = TEXT(" (15x Multisample)"); break;
		case D3DMULTISAMPLE_16_SAMPLES: pstrMultiSample = TEXT(" (16x Multisample)"); break;
		default:						pstrMultiSample = TEXT(""); break;
		}
		
		const int cchMaxFrameStats = sizeof(mFrameStats) / sizeof(TCHAR);
		_sntprintf( mFrameStats, cchMaxFrameStats, _T("%.02f fps (%dx%d), %s%s%s"), mFPS,
			mBackBuffer.Width, mBackBuffer.Height,
			strFmt, strDepthFmt, pstrMultiSample );
		mFrameStats[cchMaxFrameStats - 1] = TEXT('\0');
	}
}

/** Called in to toggle the pause state of the app. */
void CD3DApplication::pause( bool pause )
{
	static DWORD pausedCount = 0;
	
	pausedCount += (pause ? +1 : -1);
	mActive = (pausedCount ? false : true);
	
	// Handle the first pause request (of many, nestable pause requests)
	if( pause && ( 1 == pausedCount ) ) {
		// stop the timer
		CSystemTimer::getInstance().stop();
	}
	
	if( 0 == pausedCount ) {
		// start the timer
		CSystemTimer::getInstance().start();
	}
}

void CD3DApplication::getOutOfFullscreen()
{
	if( mWindowed )
		return;

	mWindowed = false;
	adjustWindowForChange();
	
	if( mD3DDevice != NULL ) {
		/*
		if( mDeviceObjectsRestored ) {
			mDeviceObjectsRestored = false;
			internalPassivateDevice();
		}
		if( mDeviceObjectsInited ) {
			mDeviceObjectsInited = false;
			internalDeleteDevice();
		}
		*/
		
		mD3DDevice->Release();
		mD3DDevice = NULL;
	}
}

/** Cleanup scene objects. */
void CD3DApplication::cleanup3DEnvironment()
{
	if( mD3DDevice != NULL ) {
		if( mDeviceObjectsRestored ) {
			mDeviceObjectsRestored = false;
			internalPassivateDevice();
		}
		if( mDeviceObjectsInited ) {
			mDeviceObjectsInited = false;
			internalDeleteDevice();
		}
		
		if( mD3DDevice->Release() > 0 )
			displayErrorMsg( NONZEROREFCOUNT, APPMUSTEXIT );
		mD3DDevice = NULL;
	}
}



void CD3DApplication::ssChangePassword()
{
	// Load the password change DLL
	HINSTANCE mpr = LoadLibrary( TEXT("MPR.DLL") );
	if( mpr != NULL ) {
		// Grab the password change function from it
		typedef DWORD (PASCAL *PWCHGPROC)( LPCSTR, HWND, DWORD, LPVOID );
		PWCHGPROC pwd = (PWCHGPROC)GetProcAddress( mpr, "PwdChangePasswordA" );
		// Do the password change
		if ( pwd != NULL )
			pwd( "SCRSAVE", mSSHwndParent, 0, NULL );
		// Free the library
		FreeLibrary( mpr );
	}
}



/** Displays error messages in a message box. */
HRESULT CD3DApplication::displayErrorMsg( HRESULT hr, eAppMsg type )
{
	static bool fatalErrorReported = false;
	TCHAR strMsg[2048];
	
	// If a fatal error message has already been reported, the app
	// is already shutting down, so don't show more error messages.
	if( fatalErrorReported )
		return hr;
	
	switch( hr ) {
	case CANTTOGGLEFULLSCREEN:
		_tcscpy( strMsg,
			_T("Could not toggle fullscreen mode.\n")
			_T("Your Direct3D device or desktop settings\n")
			_T("might not support required video mode.\n")
			_T("Try increasing desktop bit depth.\n") );
		break;

	case NODIRECT3D:
		_tcscpy( strMsg,
			_T("Could not initialize Direct3D. You may\n")
			_T("want to check that the latest version of\n")
			_T("DirectX is correctly installed on your\n")
			_T("system.\nThis application requires DirectX 9.0c") );
		break;
		
	case NOCOMPATIBLEDEVICES:
		{
			int i;
			std::string msg;
			msg  = "Could not find any compatible Direct3D devices.\n\n";
			msg += "Devices present on the system:\n";
			int n = mEnumeration.mIncompatAdapterInfos.size();
			for( i = 0; i < n; ++i ) {
				const SD3DAdapterInfo& adapter = *mEnumeration.mIncompatAdapterInfos[i];
				msg += "  ";
				msg += adapter.adapterID.Description;
				msg += "\n";
				CD3DEnumErrors::TStringSet::const_iterator it, itEnd = adapter.errors.getErrors().end();
				for( it = adapter.errors.getErrors().begin(); it != itEnd; ++it ) {
					const std::string& err = *it;
					msg += "    Error: ";
					msg += err;
					msg += "\n";
				}
			}
			n = mEnumeration.mAdapterInfos.size();
			for( i = 0; i < n; ++i ) {
				const SD3DAdapterInfo& adapter = *mEnumeration.mAdapterInfos[i];
				msg += "  ";
				msg += adapter.adapterID.Description;
				msg += "\n";
				CD3DEnumErrors::TStringSet::const_iterator it, itEnd = adapter.errors.getErrors().end();
				for( it = adapter.errors.getErrors().begin(); it != itEnd; ++it ) {
					const std::string& err = *it;
					msg += "    Error: ";
					msg += err;
					msg += "\n";
				}
			}
			strncpy( strMsg, msg.c_str(), 2047 );
		}
		break;
		
	case RESETFAILED:
		_tcscpy( strMsg, _T("Could not reset the Direct3D device." ) );
		break;
		
	case NONZEROREFCOUNT:
		_tcscpy( strMsg,
			_T("A D3D object has a non-zero reference\n")
			_T("count (meaning things were not properly\n")
			_T("cleaned up).") );
		break;
		
	case NULLREFDEVICE:
		_tcscpy( strMsg,
			_T("Warning: Nothing will be rendered.\n")
			_T("The reference rendering device was selected, but your\n")
			_T("computer only has a reduced-functionality reference device\n")
			_T("installed.  Install the DirectX SDK to get the full\n")
			_T("reference device.\n") );
		break;
		
	case E_OUTOFMEMORY:
		_tcscpy( strMsg, _T("Not enough memory.") );
		break;
		
	case D3DERR_OUTOFVIDEOMEMORY:
		_tcscpy( strMsg, _T("Not enough video memory.") );
		break;
		
	default:
		_tcscpy( strMsg, _T("Generic application error.") );
	}
	
	if( APPMUSTEXIT == type ) {
		fatalErrorReported = true;
		_tcscat( strMsg, _T("\n\nThe program will now exit.") );
		MessageBox( NULL, strMsg, mWindowTitle, MB_ICONERROR|MB_OK );
		
		// Close the window, which shuts down the app
		if( mHWnd )
			SendMessage( mHWnd, WM_CLOSE, 0, 0 );
	} else {
		if( SWITCHEDTOREF == type )
			_tcscat( strMsg,
			_T("\n\nSwitching to the reference rasterizer,\n")
			_T("a software device that implements the entire\n")
			_T("Direct3D feature set, but runs very slowly.") );
		MessageBox( NULL, strMsg, mWindowTitle, MB_ICONWARNING|MB_OK );
	}
	
	return hr;
}


// --------------------------------------------------------------------------

static INT_PTR CALLBACK gScreenSettingsDlgProcStub( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	// TBD
	//return gD3DApp->ssScreenSettingsDlgProc( hwnd, msg, wParam, lParam );
}

void CD3DApplication::ssScreenSettingsDlg( HWND hwndParent )
{
	// TBD
	//LPCTSTR pstrTemplate = MAKEINTRESOURCE( IDD_SINGLEMONITORSETTINGS );
	//DialogBox(mInstance, pstrTemplate, hwndParent, screenSettingsDlgProcStub );
}

