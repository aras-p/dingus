// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef D3D_APPLICATION_H__
#define D3D_APPLICATION_H__

#include "D3DEnumeration.h"
#include "D3DSettings.h"


namespace dingus {
	
	

//---------------------------------------------------------------------------
// error codes

enum eAppMsg { NONE = 0, APPMUSTEXIT, SWITCHEDTOREF };

enum eAppErr {
	NODIRECT3D			= 0x82000001,
	NOCOMPATIBLEDEVICES = 0x82000003,
	NONZEROREFCOUNT 	= 0x8200000a,
	RESETFAILED 		= 0x8200000c,
	NULLREFDEVICE		= 0x8200000d,
	CANTTOGGLEFULLSCREEN= 0x8200000e
};


// --------------------------------------------------------------------------
//  screensaver stuff

// Modes of operation for screensaver
enum eSaverMode {
	SM_NONE,		// No screensaver
	SM_CONFIG,		// Config dialog box
	SM_PREVIEW, 	// Mini preview window in Display Properties dialog
	SM_FULL,		// Full-on screensaver mode
	SM_TEST,		// Test mode - windowed
	SM_PASSCHANGE	// Change password
};

// Prototype for VerifyScreenSavePwd() in password.cpl, used on Win9x
typedef BOOL (PASCAL* SSVERIFYPASSWORDPROC)(HWND);


//---------------------------------------------------------------------------

class CD3DApplication : public boost::noncopyable {
protected:
	CD3DEnumeration		mEnumeration;
	CD3DSettings		mSettings;
	SD3DSettingsPref	mSettingsPref;

	// internal variables for screensaver mode only
	eSaverMode	mSSMode;
	bool		mSSInStartingPause;
	UINT		mSSMouseMoveCount;
	bool		mSSIsWin9x;
	HINSTANCE	mSSPasswordDLL;
	SSVERIFYPASSWORDPROC	mSSVerifyPasswordProc;
	bool		mSSCheckingPassword;
	TCHAR		mSSRegistryPath[200];
	HWND		mSSHwndParent;
	
	// internal variables for the state of the app
	bool		mWindowed;
	bool		mActive;
	bool		mDeviceLost;
	bool		mMinimized;
	bool		mMaximized;
	bool		mIgnoreSizeChange;
	bool		mDeviceObjectsInited;
	bool		mDeviceObjectsRestored;
	
private:
	eSaverMode	ssParseCmdLine( TCHAR* cmdLine );
	void		ssChangePassword();
	HRESULT		ssDoSaver();
	void		ssReadSettings();
	void		ssWriteSettings();
	void		ssDoPaint( HWND hwnd, HDC hdc );
	void		ssInterrupt();
	void		ssScreenSettingsDlg( HWND wndParent );
	INT_PTR 	ssScreenSettingsDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void		ssSetupAdapterPage( HWND hwnd );
	virtual void ssDoConfig() { } // overriden by the app
	
protected:
	// internal error handling function
	HRESULT displayErrorMsg( HRESULT hr, eAppMsg type );
	
	// internal functions to manage and render the 3D scene

	// return false on error
	bool	buildPresentParamsFromSettings();
	bool	findBestWindowedMode( bool requireHAL, bool requireREF );
	bool	findBestFullscreenMode( bool requireHAL, bool requireREF );
	
	virtual HRESULT chooseInitialD3DSettings();
	HRESULT initialize3DEnvironment();
	HRESULT handlePossibleSizeChange();
	HRESULT reset3DEnvironment();
	HRESULT toggleFullscreen();
	HRESULT forceWindowed();

	HRESULT applySettings();
	HRESULT userSelectNewDevice();
	/// Return false if cancelled.
	bool	justShowSettingsDialog();
	
	void	cleanup3DEnvironment();
	HRESULT render3DEnvironment();
	virtual HRESULT adjustWindowForChange();
	virtual void updateStats();

private:
	HRESULT	internalCreateDevice();
	HRESULT	internalDeleteDevice();
	HRESULT internalActivateDevice();
	HRESULT internalPassivateDevice();

	void	modifyD3DSettingsFromPref();

protected:
	// main objects used for creating and rendering the 3D scene
	D3DPRESENT_PARAMETERS mPresentParams;  // Parameters for CreateDevice/Reset
	HINSTANCE			mHInstance;
	HWND				mHWnd;				// The main app window
	HWND				mHWndFocus; 		// The D3D focus window (usually same as m_hWnd)
	HMENU				mHMenu; 			// App menu bar (stored here when fullscreen)
	IDirect3D9* 		mD3D;			   // The main D3D object
	IDirect3DDevice9*	mD3DDevice; 	   // The D3D rendering device
	D3DCAPS9			mD3DCaps;			// Caps for the device
	D3DSURFACE_DESC 	mBackBuffer;   // Surface desc of the backbuffer
	DWORD				mCreateFlags;	  // Indicate sw or hw vertex processing
	DWORD				mWindowStyle;	  // Saved window style for mode switches
	RECT				mWindowBounds;	  // Saved window bounds for mode switches
	RECT				mWindowClient;	  // Saved client area size for mode switches
	
	// variables for timing
	float	mFPS;			   // Instanteous frame rate
	TCHAR	mDeviceStats[90];// String to hold D3D device stats
	TCHAR	mFrameStats[90]; // String to hold frame stats
	
	// overridable variables for the app
	const TCHAR*	mWindowTitle;    // Title for the app's window
	DWORD			mCreationWidth;   // Width used to create window
	DWORD			mCreationHeight;  // Height used to create window
	bool			mShowCursorWhenFullscreen; // Whether to show cursor when fullscreen
	bool			mClipCursorWhenFullscreen; // Whether to limit cursor pos when fullscreen
	bool			mStartFullscreen;  // Whether to start up the app in fullscreen mode
	bool			mVSyncFullscreen;
	bool			mSelectDeviceAtStartup;
	bool			mDebugTimer; // Constant step timer
	
	static bool checkDeviceHelper( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors );

	// overridable functions for the 3D scene created by the app
	virtual bool checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors ) /*{ outMessage=""; return true; } TBD */ = 0;
	virtual HRESULT initialize()						   { return S_OK; }
	virtual HRESULT createDeviceObjects() 					   { return S_OK; }
	virtual HRESULT activateDeviceObjects()					   { return S_OK; }
	//virtual HRESULT frameMove() 							   { return S_OK; }
	virtual HRESULT passivateDeviceObjects()				   { return S_OK; }
	virtual HRESULT deleteDeviceObjects()					   { return S_OK; }
	virtual HRESULT shutdown()							   { return S_OK; }
	
public:
	void	getOutOfFullscreen();

	virtual HRESULT performOneTime() { return S_OK; }

	// override for custom settings dialogs
	virtual void customSettingsInit( HWND dlg ) { };
	virtual void customSettingsOK( HWND dlg ) { };
	
	// functions to create, run, pause, and clean up the application
	virtual HRESULT create( HINSTANCE hInstance, bool screenSaverMode );
	virtual int 	run();
	virtual LRESULT msgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void	pause( bool bPause );
	bool			isActive() const { return mActive; }
	void			close();
	void			doClose();
	virtual 		~CD3DApplication() { }
	
	CD3DApplication();
};
	
}; // namespace dingus



#endif
