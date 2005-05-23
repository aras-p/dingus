#include "stdafx.h"
#include "System.h"

#include <dingus/lua/LuaSingleton.h>
#include <dingus/gfx/gui/Gui.h>

#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>

#include <dingus/console/W32StdConsoleRenderingContext.h>
#include <dingus/console/WDebugConsoleRenderingContext.h>

#include <dingus/input/DIKeyboard.h>
#include <dingus/input/DIMouse.h>


const bool DEV_MODE = false;


SAppStartupParams CSystem::getStartupParams()
{
	SAppStartupParams sp;
	sp.windowTitle = "in.out.side: the shell";
	sp.dataPath = "data/";
	sp.windowWidth = 640;
	sp.windowHeight = 480;
	sp.minColorBits = 8;
	sp.minAlphaBits = 8;
	sp.minZBits = 16;
	sp.minStencilBits = 0;
	sp.usesZBuffer = true;
	sp.startFullscreen = !DEV_MODE;
	//sp.startFullscreen = true;
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
	CLuaSingleton::init( "" );
	CTextureBundle::getInstance().addDirectory( dataPath + "tex/" );
	CCubeTextureBundle::getInstance().addDirectory( dataPath + "tex/" );
	CMeshBundle::getInstance().addDirectory( dataPath + "mesh/" );
	CSkeletonInfoBundle::getInstance().addDirectory( dataPath + "mesh/" );
	CEffectBundle::getInstance().addDirectory( dataPath + "fx/" );
	CAnimationBundle::getInstance().addDirectory( dataPath + "anim/" );
	CModelDescBundle::getInstance().addDirectory( dataPath + "model/" );

	CDynamicVBManager::initialize( 3 * 1024 * 1024 );
	CDynamicVBManager& vbManager = CDynamicVBManager::getInstance();
	CDynamicIBManager::initialize( (1024+512) * 1024 );
	CDynamicIBManager& ibManager = CDynamicIBManager::getInstance();

	//
	// device dependant resources

	CDeviceResourceManager& deviceManager = CDeviceResourceManager::getInstance();
	deviceManager.addListener( CVertexDeclBundle::getInstance() );
	deviceManager.addListener( CSharedTextureBundle::getInstance() );
	deviceManager.addListener( CSharedSurfaceBundle::getInstance() );
	deviceManager.addListener( vbManager );
	deviceManager.addListener( ibManager );
	deviceManager.addListener( CTextureBundle::getInstance() );
	deviceManager.addListener( CCubeTextureBundle::getInstance() );
	deviceManager.addListener( CSharedMeshBundle::getInstance() );
	deviceManager.addListener( CMeshBundle::getInstance() );
	deviceManager.addListener( CSkinMeshBundle::getInstance() );
	deviceManager.addListener( CEffectBundle::getInstance() );
	deviceManager.addListener( CIndexBufferBundle::getInstance() );
	CUIResourceManager::initialize( GUI_X, GUI_Y );
	deviceManager.addListener( CUIResourceManager::getInstance() );
}

void CSystem::setupContexts( HWND hwnd )
{
	mHwnd = hwnd;
	
	// renderer
	assert( !G_RENDERCTX );
	G_RENDERCTX = new CRenderContext( *RGET_FX("lib/global") );

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
	assert( G_RENDERCTX );
	safeDelete( G_RENDERCTX );

	assert( G_INPUTCTX );
	stl_utils::wipe( G_INPUTCTX->getDevices() );
	safeDelete( G_INPUTCTX );
}

void CSystem::destroyBundles()
{
	CDynamicVBManager::finalize();
	CDynamicIBManager::finalize();

	CAnimationBundle::finalize();
	CCubeTextureBundle::finalize();
	CEffectBundle::finalize();
	CIndexBufferBundle::finalize();
	CMeshBundle::finalize();
	CSkinMeshBundle::finalize();
	CModelDescBundle::finalize();
	CSharedMeshBundle::finalize();
	CSharedTextureBundle::finalize();
	CSharedSurfaceBundle::finalize();
	CSkeletonInfoBundle::finalize();
	CTextureBundle::finalize();
	CVertexDeclBundle::finalize();
	
	CLuaSingleton::finalize();
	CUIResourceManager::finalize();
}
