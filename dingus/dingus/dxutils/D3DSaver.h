// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __D3D_SAVER_H
#define __D3D_SAVER_H

namespace dingus {
namespace ss {


// --------------------------------------------------------------------------
//  Error codes

enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D		  0x82000001
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOHARDWAREDEVICE	  0x82000005
#define D3DAPPERR_INITDEVICEOBJECTSFAILED 0x8200000d
#define D3DAPPERR_CREATEDEVICEFAILED  0x8200000e
#define D3DAPPERR_NOPREVIEW 		  0x8200000f


// --------------------------------------------------------------------------
//  Constants

#define MAX_DISPLAYS 9
#define NO_ADAPTER 0xffffffff
//#define NO_MONITOR 0xffffffff


// Modes of operation for screensaver
enum eSaverMode {
	SM_CONFIG,		// Config dialog box
	SM_PREVIEW, 	// Mini preview window in Display Properties dialog
	SM_FULL,		// Full-on screensaver mode
	SM_TEST,		// Test mode
	SM_PASSCHANGE	// Change password
};


// Prototype for VerifyScreenSavePwd() in password.cpl, used on Win9x
typedef bool (PASCAL * SSVERIFYPASSWORDPROC) (HWND);


// --------------------------------------------------------------------------


/// Structure for holding information about a display mode.
struct SD3DModeInfo {
	DWORD	   width;		// Screen width in this mode
	DWORD	   height;		// Screen height in this mode
	D3DFORMAT  format;		// Pixel format in this mode
	DWORD	   behavior;	// Hardware / Software / Mixed vertex processing
	D3DFORMAT  dsformat;	// Which depth/stencil format to use with this mode
};


/// Structure for holding information about a display mode.
struct SD3DWindowModeInfo {
	D3DFORMAT  dispFormat;
	D3DFORMAT  bbFormat;
	DWORD	   behavior;	// Hardware / Software / Mixed vertex processing
	D3DFORMAT  dsformat;	// Which depth/stencil format to use with this mode
};


/// Holds information about a D3D device and list of modes.
struct SD3DDeviceInfo {
	// Device data
	D3DDEVTYPE	 devType;
	D3DCAPS9	 caps;
	const TCHAR* desc;
	bool		 canDoWindowed; // Can work in windowed mode?
	
	DWORD		 modeCount;
	SD3DModeInfo modes[150];
	
	// Current state
	DWORD		 currMode;
	bool		 windowed;
	D3DMULTISAMPLE_TYPE multiSampleType;
};


/// Information about an adapter, and list of devices on it.
struct SD3DAdapterInfo {
public:
	// Adapter data
	//DWORD		   monIdx; // Which SMonitorInfo corresponds to this adapter
	D3DADAPTER_IDENTIFIER9 adapterID;
	D3DDISPLAYMODE desktopDM;	  // Desktop display mode for this adapter
	
	// Devices for this adapter
	DWORD		   deviceCount;
	SD3DDeviceInfo devices[3];
	bool		   hasHAL;
	bool		   hasNeededHAL;
	bool		   hasSW;
	bool		   hasNeededSW;
	
	// User's preferred mode settings for this adapter
	DWORD		   userPrefWidth;
	DWORD		   userPrefHeight;
	D3DFORMAT	   userPrefFormat;
	bool		   disableHAL;	 // If true, don't use HAL on this display
	
	// Current state
	DWORD		   currDevice;
	//HWND		   hwndDevice;
};



/// Information about a monitor.
/*
struct SMonitorInfo {
	TCHAR		  deviceName[128];
	TCHAR		  monitorName[128];
	HMONITOR	  hMonitor;
	RECT		  screenRect;
	DWORD		  adapterIdx; // Which SD3DAdapterInfo corresponds to this monitor
	HWND		  hwnd;
	
};
*/


/*
struct SRenderUnit {
	UINT				  adapterIdx;
	UINT				  monIdx;
	D3DDEVTYPE			  devType;
	DWORD				  behavior;
	IDirect3DDevice9*	  device;
	D3DPRESENT_PARAMETERS presentParams;
	bool				  devObjsInited;	// createDeviceObjects was called
	bool				  devObjsRestored;	// activateDeviceObjects was called
	TCHAR				  deviceStats[90];	// String to hold D3D device stats
	TCHAR				  frameStats[40];	// String to hold frame stats
};

*/


// --------------------------------------------------------------------------

/// D3D screensaver class
class CD3DScreensaver {
public:
	CD3DScreensaver();
	
	virtual HRESULT create( HINSTANCE hInstance );
	virtual INT 	run();
	HRESULT 		displayErrorMsg( HRESULT hr, DWORD type = 0 );
	
protected:
	eSaverMode		parseCmdLine( TCHAR* cmdLine );
	void			changePassword();
	HRESULT 		doSaver();
	
	virtual void	doConfig() { }
	virtual void	readSettings() { };
	void			readScreenSettings( HKEY hkeyParent );
	void			writeScreenSettings( HKEY hkeyParent );
	
	virtual void	doPaint( HWND hwnd, HDC hdc );
	HRESULT 		initialize3DEnvironment();
	void			cleanup3DEnvironment();
	HRESULT 		render3DEnvironment();
	static LRESULT CALLBACK saverProcStub( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	virtual LRESULT saverProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void			interruptSaver();
	void			shutdownSaver();
	void			doScreenSettingsDlg( HWND hwndParent );
	static INT_PTR CALLBACK screenSettingsDlgProcStub( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	INT_PTR 		screenSettingsDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void			setupAdapterPage( HWND hwnd );
	
	HRESULT 		createSaverWindow();
	HRESULT 		buildDeviceList();
	bool			findDepthStencilFormat( UINT adapterIdx, D3DDEVTYPE devType, D3DFORMAT targetFmt, D3DFORMAT* depthStencilFmt );
	HRESULT 		checkWindowedFormat( UINT adapterIdx, SD3DWindowModeInfo* windowModeInfo );
	HRESULT 		createFullscreen();
	HRESULT 		createWindowed();
	bool			findNextLowerMode( SD3DDeviceInfo* devInfo );
	void			switchToRenderUnit( UINT ru );
	void			buildProjMatrix( float nearPlane, float farPlane, D3DXMATRIX* matrix );
	HRESULT 		setProjectionMatrix( float nearPlane, float farPlane );
	virtual void	updateDeviceStats();
	virtual void	updateFrameStats();
	virtual bool	getTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );
	void			updateErrorBox();
	//void			enumMonitors( void );
	bool			getBestAdapter( UINT* adapter );
	
	virtual void	setDevice( UINT deviceIdx )				   { }
	virtual HRESULT registerSWDevice()				   { return S_OK; }
	virtual HRESULT confirmDevice( D3DCAPS9* caps, DWORD behavior, D3DFORMAT backBufferFmt ) { return S_OK; }
	virtual HRESULT confirmMode( LPDIRECT3DDEVICE9 pd3dDev )   { return S_OK; }

	virtual HRESULT initialize()					{ return S_OK; }
	virtual HRESULT createDeviceObjects() 			{ return S_OK; }
	virtual HRESULT activateDeviceObjects()			{ return S_OK; }
	virtual HRESULT performLogic()					{ return S_OK; }
	virtual HRESULT performRender()					{ return S_OK; }
	virtual HRESULT passivateDeviceObjects()		{ return S_OK; }
	virtual HRESULT deleteDeviceObjects()			{ return S_OK; }
	virtual HRESULT shutdown()						{ return S_OK; }
	
protected:
	eSaverMode		mSaverMode;		 // SM_CONFIG, SM_FULL, SM_PREVIEW, etc.
	//bool			mAllScreensSame;	 // If true, show same image on all screens
	HWND			mHwnd; 			 // Focus window and device window on primary
	HWND			mHwndParent;
	HINSTANCE		mInstance;
	bool			mInStartingPause;  // Used to pause when preview starts
	UINT			mMouseMoveCount;
	bool			mIsWin9x;
	HINSTANCE		mPasswordDLL;
	SSVERIFYPASSWORDPROC	mVerifyPasswordProc;
	bool			mCheckingPassword;
	bool			mWindowed;
	
	// Variables for non-fatal error management
	bool			mErrorMode;		 // Whether to display an error
	HRESULT 		mErrorHR;			 // Error code to display
	TCHAR			mErrorText[400]; 	 // Error message text
	float			mErrorX;
	float			mErrorY;
	float			mErrorWidth;
	float			mErrorHeight;
	float			mErrorXVel;
	float			mErrorYVel;
	
	//UINT			mMonitorCount;
	//SMonitorInfo 	mMonitors[MAX_DISPLAYS];
	//DWORD			mRenderUnitCount;
	//SRenderUnit		mRenderUnits[MAX_DISPLAYS];

	SD3DAdapterInfo*	mAdapters[MAX_DISPLAYS];
	UINT				mAdapterCount;

	// current settings
	IDirect3D9*			mD3D;
	IDirect3DDevice9*	mD3DDevice;
	UINT				mCurrAdapterIdx; // current adapter
	//UINT				mCurrMonitorIdx;
	D3DDEVTYPE			mCurrDevType;
	DWORD				mCurrBehavior;
	D3DPRESENT_PARAMETERS mCurrPresentParams;
	bool				mDevObjsInited;	// createDeviceObjects was called
	bool				mDevObjsRestored;	// activateDeviceObjects was called
	//TCHAR				mCurrDeviceStats[90];	// String to hold D3D device stats
	//TCHAR				mCurrFrameStats[40];	// String to hold frame stats

	//RECT			mRenderAreaTotal;	 // Entire area to be rendered
	//RECT			mRenderAreaCurr;	 // performOneTime area of current device
	D3DSURFACE_DESC mBackBufferDesc;	 // Info on back buffer for current device
	
	TCHAR			mWindowTitle[200];	// Title for the app's window
	bool			mAllowRef;			// Whether to allow REF D3D device
	bool			mUseDepthBuffer;	// Whether to autocreate depthbuffer
	bool			mMultithreaded;		// Whether to make D3D thread-safe
	//bool			mOneScreenOnly;		// Only ever show screensaver on one screen
	TCHAR			mRegistryPath[200];	// Where to store registry info
	UINT			mMinDepthBits;		// Minimum number of bits needed in depth buffer
	UINT			mMinStencilBits;	// Minimum number of bits needed in stencil buffer
	D3DSWAPEFFECT	mSwapFxFullscreen;	// SwapEffect to use in fullscreen Present()
	D3DSWAPEFFECT	mSwapFxWindowed;	// SwapEffect to use in windowed Present()
	
	// Variables for timing
	float	mTime;				// Current time in seconds
	float	mElapsedTime; 		// Time elapsed since last frame
	float	mFPS; 				// Instanteous frame rate
	TCHAR	mDeviceStats[90];	// D3D device stats for current device
	TCHAR	mFrameStats[40];	// Frame stats for current device
};


}; // namespace ss
}; // namespace dingus

#endif
