#include "stdafx.h"
#include "System.h"

#include <dingus/gfx/gui/Gui.h>

#include <dingus/console/W32StdConsoleRenderingContext.h>
#include <dingus/console/WDebugConsoleRenderingContext.h>

#include <dingus/input/DIKeyboard.h>
#include <dingus/input/DIMouse.h>


const bool DEV_MODE = true;


SAppStartupParams CSystem::getStartupParams()
{
	SAppStartupParams sp;
	sp.windowTitle = "VaikaiRoks TBD";
	sp.dataPath = "data/";
	sp.windowWidth = 640;
	sp.windowHeight = 480;
	sp.minColorBits = 4;
	sp.minAlphaBits = 4;
	sp.minZBits = 16;
	sp.minStencilBits = 0;
	sp.usesZBuffer = true;
	sp.startFullscreen = !DEV_MODE;
	sp.showCursorFullscreen = true;
	sp.vsyncFullscreen = !DEV_MODE;
	sp.debugTimer = false;
	sp.selectDeviceAtStartup = !DEV_MODE;
	return sp;
}

IConsoleRenderingContext* CSystem::createStdConsoleCtx( HWND hwnd )
{
	if( DEV_MODE )
		return new CW32StdConsoleRenderingContext();
	else
		return NULL;
}


void CSystem::setupBundles( const std::string& dataPath, dingus::CReloadableBundleManager& reloadManager )
{
	CTextureBundle::getInstance().addDirectory( dataPath + "tex/" );

	//
	// device dependant resources

	CDeviceResourceManager& deviceManager = CDeviceResourceManager::getInstance();
	deviceManager.addListener( CVertexDeclBundle::getInstance() );
	deviceManager.addListener( CTextureBundle::getInstance() );
	CUIResourceManager::initialize( GUI_X, GUI_Y );
	deviceManager.addListener( CUIResourceManager::getInstance() );
}

void CSystem::setupContexts( HWND hwnd )
{
	mHwnd = hwnd;
	
	// input devices
	assert( !G_INPUTCTX );
	G_INPUTCTX = new CInputContext();

	HRESULT hr;

	IDirectInput8* directInput8 = NULL;
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput8, NULL );
	assert( SUCCEEDED( hr ) );
	assert( directInput8 );
	G_INPUTCTX->addDevice( *(new CDIKeyboard(hwnd,*directInput8)) );
	G_INPUTCTX->addDevice( *(new CDIMouse(hwnd,*directInput8)) );
}


void CSystem::destroyContexts()
{
	assert( G_INPUTCTX );
	safeDelete( G_INPUTCTX );
}

void CSystem::destroyBundles()
{
	CTextureBundle::finalize();
	CVertexDeclBundle::finalize();
	
	CUIResourceManager::finalize();
}
