// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DINGUS_APPLICATION_H
#define __DINGUS_APPLICATION_H

#include "../console/Console.h"
#include "../kernel/D3DDeviceCaps.h"

namespace dingus {

class CReloadableBundleManager;


// --------------------------------------------------------------------------

struct SAppStartupParams {
public:
	SAppStartupParams()
		: windowTitle("???"), dataPath(""), dxConfigFile(NULL),
		windowWidth(320), windowHeight(240), minColorBits(4), minAlphaBits(4),
		minZBits(16), minStencilBits(0), usesZBuffer(true),
		startFullscreen(false), showCursorFullscreen(false),
		vsyncFullscreen(false), debugTimer(false),
		selectDeviceAtStartup(false)
	{
	}

public:
	const char*		windowTitle;
	const char*		dataPath;
	const WCHAR*	dxConfigFile;
	int		windowWidth;
	int		windowHeight;
	int		minColorBits;
	int		minAlphaBits;
	int		minZBits;
	int		minStencilBits;
	bool	usesZBuffer;
	bool	startFullscreen;
	bool	showCursorFullscreen;
	bool	vsyncFullscreen;
	bool	debugTimer; // constant dt timer

	bool	selectDeviceAtStartup;
};

// --------------------------------------------------------------------------

class CD3DEnumeration;
class CD3DSettings;
struct SD3DSettingsPref;



class IDingusAppContext {
public:
	virtual const CD3DEnumeration&	getD3DEnumeration() const = 0;
	virtual const CD3DSettings&		getD3DSettings() const = 0;
	virtual void applyD3DSettings( const CD3DSettings& s ) = 0;
};


// --------------------------------------------------------------------------



class IDingusApplication {
public:
	virtual ~IDingusApplication() = 0 { }

	virtual SAppStartupParams getStartupParams() = 0;
	virtual bool checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors ) = 0;
	virtual void initD3DSettingsPref( SD3DSettingsPref& pref ) { }

	virtual void initialize( IDingusAppContext& appContext ) = 0;
	virtual void shutdown() = 0;

	/**
	 *  Constantly polled, if returns true - app ends.
	 */
	virtual bool shouldFinish() = 0;

	/**
	 *  Constantly polled, if returs true - show render stats.
	 */
	virtual bool shouldShowStats() = 0;

	/**
	 *  Called to execute all main loop pipeline. It's application task
	 *  to process contexts in needed order, etc.
	 */
	virtual void perform() = 0;

	// console related stuff
	virtual dingus::IConsoleRenderingContext* createStdConsoleCtx( HWND hwnd ) = 0;

	// setup stuff
	virtual void setupBundles( const std::string& dataPath, dingus::CReloadableBundleManager& reloadManager ) = 0;
	virtual void setupContexts( HWND hwnd ) = 0;
	virtual void destroyContexts() = 0;
	virtual void destroyBundles() = 0;

	/**
	 *  Windows message processing.
	 *  @return True if the message was processed.
	 */
	virtual bool msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) { return false; }
};



}; // namespace

#endif
