#include "stdafx.h"
#include "System.h"

#include <dingus/lua/LuaSingleton.h>
#include <dingus/gfx/gui/Gui.h>

#include <dingus/gfx/geometry/DynamicVBManager.h>

//#include <dingus/console/W32StdConsoleRenderingContext.h>
//#include <dingus/console/WDebugConsoleRenderingContext.h>
#include <dingus/console/FileConsoleRenderingContext.h>

#include <dingus/input/DIKeyboard.h>
#include <dingus/input/DIMouse.h>


const bool DEV_MODE = true;


SAppStartupParams CSystem::getStartupParams()
{
	SAppStartupParams sp;
	sp.windowTitle = "Project Hoshimi 3D Viewer";
	sp.dataPath = "data/";
	sp.windowWidth = 640;
	sp.windowHeight = 480;
	sp.minColorBits = 4;
	sp.minAlphaBits = 0;
	sp.minZBits = 16;
	sp.minStencilBits = 4;
	sp.usesZBuffer = true;
	sp.startFullscreen = !DEV_MODE;
	sp.showCursorFullscreen = true;
	sp.vsyncFullscreen = !DEV_MODE;
	sp.debugTimer = false;
	sp.selectDeviceAtStartup = !DEV_MODE;
	sp.dxConfigFile = L"data/directxdb.txt";
	return sp;
}

IConsoleRenderingContext* CSystem::createStdConsoleCtx( HWND hwnd )
{
	CFileConsoleRenderingContext* ctx = new CFileConsoleRenderingContext( "PHViewer3D_log.txt" );
	return ctx;
}

static std::string gSavedDataPath;

void CSystem::setupBundles( const std::string& dataPath, dingus::CReloadableBundleManager& reloadManager )
{
	gSavedDataPath = dataPath;

	CLuaSingleton::init( "" );
	CTextureBundle::getInstance().addDirectory( dataPath + "tex/" );
	CCubeTextureBundle::getInstance().addDirectory( dataPath + "tex/" );
	CMeshBundle::getInstance().addDirectory( dataPath + "mesh/" );
	CEffectBundle::getInstance().addDirectory( dataPath + "fx/" );
	CEffectBundle::getInstance().setStatesConfig( (dataPath + "EffectStates.lua").c_str() );

	CAnimationBundle::getInstance().addDirectory( dataPath + "anim/" );
	CModelDescBundle::getInstance().addDirectory( dataPath + "model/" );
	//CFontBundle::getInstance().addDirectory( dataPath );

	CDynamicVBManager::initialize( 512 * 1024 ); // 512 kilobytes
	CDynamicVBManager& vbManager = CDynamicVBManager::getInstance();

	//
	// device dependant resources

	CDeviceResourceManager& deviceManager = CDeviceResourceManager::getInstance();
	deviceManager.addListener( CVertexDeclBundle::getInstance() );
	deviceManager.addListener( CSharedTextureBundle::getInstance() );
	deviceManager.addListener( CSharedSurfaceBundle::getInstance() );
	deviceManager.addListener( vbManager );
	deviceManager.addListener( CTextureBundle::getInstance() );
	deviceManager.addListener( CCubeTextureBundle::getInstance() );
	deviceManager.addListener( CSharedMeshBundle::getInstance() );
	deviceManager.addListener( CMeshBundle::getInstance() );
	deviceManager.addListener( CEffectBundle::getInstance() );
	deviceManager.addListener( CIndexBufferBundle::getInstance() );
	CUIResourceManager::initialize( GUI_X, GUI_Y );
	deviceManager.addListener( CUIResourceManager::getInstance() );
}

void CSystem::setupContexts( HWND hwnd )
{
	mHwnd = hwnd;
	
	// renderer
	CONS << "create render context" << endl;
	assert( !G_RENDERCTX );
	G_RENDERCTX = new CRenderContext( *RGET_FX("lib/global") );

	// input devices
	CONS << "create input context" << endl;
	assert( !G_INPUTCTX );
	G_INPUTCTX = new CInputContext();

	HRESULT hr;

	IDirectInput8* directInput8 = NULL;
	CONS << "initialize DirectInput" << endl;
	hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput8, NULL );
	assert( SUCCEEDED( hr ) );
	assert( directInput8 );
	CONS << " add DI keyboard device" << endl;
	G_INPUTCTX->addDevice( *(new CDIKeyboard(hwnd,*directInput8)) );
	CONS << " add DI mouse device" << endl;
	G_INPUTCTX->addDevice( *(new CDIMouse(hwnd,*directInput8)) );

	// sound
	CONS << "create audio context" << endl;
	new CAudioContext( hwnd );
	CONS << "  open audio context" << endl;
	G_AUDIOCTX->open();
	CONS << "  initialize sound bundle" << endl;
	CSoundBundle::getInstance().initialize( gSavedDataPath + "sound/" );
}


void CSystem::destroyContexts()
{
	assert( G_RENDERCTX );
	safeDelete( G_RENDERCTX );

	assert( G_INPUTCTX );
	stl_utils::wipe( G_INPUTCTX->getDevices() );
	safeDelete( G_INPUTCTX );

	assert( G_AUDIOCTX );
	CSoundBundle::finalize();
	safeDelete( G_AUDIOCTX );
}

void CSystem::destroyBundles()
{
	CDynamicVBManager::finalize();

	CAnimationBundle::finalize();
	CCubeTextureBundle::finalize();
	CEffectBundle::finalize();
	//CFontBundle::finalize();
	CIndexBufferBundle::finalize();
	CMeshBundle::finalize();
	CModelDescBundle::finalize();
	CSharedMeshBundle::finalize();
	CSharedTextureBundle::finalize();
	CSharedSurfaceBundle::finalize();
	CTextureBundle::finalize();
	CVertexDeclBundle::finalize();
	
	CLuaSingleton::finalize();
	CUIResourceManager::finalize();
}
