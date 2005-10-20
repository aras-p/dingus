// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "DingusSystem.h"

#include "../kernel/D3DDevice.h"

#include "../resource/DeviceResource.h"
#include "../resource/ReloadableBundle.h"

#include "../renderer/RenderContext.h"
#include "../renderer/EffectParamsNotifier.h"

#include "../dxutils/D3DFont.h"
#include "../console/D3DConsoleRenderingContext.h"


#define APP_CHANNEL CConsole::getChannel("app")

using namespace dingus;


CDingusSystem::CDingusSystem( IDingusApplication& application )
:	mApplication( &application ),
	mAppInited( false ),
	mStdConsoleCtx(0), mFont(0), mD3DConsoleCtx(0)
{
	SAppStartupParams params = mApplication->getStartupParams();
	mCreationWidth		= params.windowWidth;
	mCreationHeight 	= params.windowHeight;
	mWindowTitle		= params.windowTitle;

	mStartFullscreen					= params.startFullscreen;
	mShowCursorWhenFullscreen			= params.showCursorFullscreen;
	mVSyncFullscreen					= params.vsyncFullscreen;
	mDebugTimer							= params.debugTimer;
	mEnumeration.mUsesDepthBuffer		= params.usesZBuffer;
	mEnumeration.mMinColorChannelBits	= params.minColorBits;
	mEnumeration.mMinAlphaChannelBits	= params.minAlphaBits;
	mEnumeration.mMinDepthBits			= params.minZBits;
	mEnumeration.mMinStencilBits		= params.minStencilBits;
	mEnumeration.mUsesMixedVP			= true;
	mEnumeration.mConfigDBFileName		= params.dxConfigFile;
	mDataPath = params.dataPath;

	mSelectDeviceAtStartup = params.selectDeviceAtStartup;

	mFont = new CD3DFont( "Arial", 10, CD3DFont::BOLD );
};


CDingusSystem::~CDingusSystem()
{
	delete mFont;
}


/**
 *  Initialization. Paired with shutdown().
 *  The window has been created and the IDirect3D9 interface has been
 *  created, but the device has not been created yet. Here you can
 *  perform application-related initialization and cleanup that does
 *  not depend on a device.
 */
HRESULT CDingusSystem::initialize()
{
	assert( mApplication );

	//
	// init console

	mStdConsoleCtx = mApplication->createStdConsoleCtx( mHWnd );
	CConsole::getInstance().setDefaultRenderingContext( *mStdConsoleCtx );

	mD3DConsoleCtx = new CD3DTextBoxConsoleRenderingContext( *mFont, 2, 2, 0xc0FFFF20, 0xc0000000 );
	CConsole::getChannel( "system" ).setRenderingContext( *mD3DConsoleCtx );

	APP_CHANNEL << "creating device resource manager" << endl;
	CDeviceResourceManager::getInstance();

	APP_CHANNEL << "creating reloadable resources manager" << endl;
	mReloadableManager = new CReloadableBundleManager();

	return S_OK;
}


bool CDingusSystem::checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	return mApplication->checkDevice( caps, vproc, errors );
}


HRESULT CDingusSystem::createDeviceObjects()
{
	mFont->createDeviceObjects();
	
	if( !mAppInited ) {
		APP_CHANNEL << "setup resource bundles" << endl;
		mApplication->setupBundles( mDataPath, *mReloadableManager );
	}

	APP_CHANNEL << "create d3d resources" << endl;
	CDeviceResourceManager::getInstance().createResource();

	CEffectParamsNotifier::getInstance().notify();

	return S_OK;
}

HRESULT CDingusSystem::activateDeviceObjects()
{
	APP_CHANNEL << "activate d3d resources" << endl;
	mFont->activateDeviceObjects();
	CDeviceResourceManager::getInstance().activateResource();

	if( !mAppInited ) {
		APP_CHANNEL << "setup app contexts" << endl;
		mApplication->setupContexts( mHWnd );
		APP_CHANNEL << "initialize app" << endl;
		mApplication->initialize( *this );
		APP_CHANNEL << "app initialized" << endl;
		mAppInited = true;
	}
	
	return S_OK;
}


/**
 *  Called once per frame, the call is the entry point for 3d rendering. This
 *  function sets up render states, clears the viewport, and renders the scene.
 */
HRESULT CDingusSystem::performOneTime()
{
	CD3DDevice& device = CD3DDevice::getInstance();

	//
	// pipeline

	device.getStats().reset();
	device.getStats().setFPS( mFPS );

	// needs to reset device cache; otherwise we can get funky things with
	// render targets
	device.resetCachedState();

	mApplication->perform();

	//
	// stats

	if( mApplication->shouldShowStats() ) {
		CConsoleChannel& cc = CConsole::getChannel( "system" );
		cc.write( mFrameStats );
		cc.write( mDeviceStats );

		const dingus::CRenderStats& stats = device.getStats();
		char buf[300];
		sprintf( buf, "draws %i, fx %i, prims %i (%.1fM/s), verts %i",
			stats.getDrawCalls(),
			stats.getEffectChanges(),
			stats.getPrimsRendered(),
			stats.getPrimsRendered() * mFPS * 1.0e-6f,
			stats.getVerticesRendered() );
		cc.write( buf );

		const dingus::CRenderStats::SStateStats& ssc = stats.changes;
		const dingus::CRenderStats::SStateStats& ssf = stats.filtered;
		sprintf( buf, "vb:%i/%i ib:%i/%i dcl:%i/%i rt:%i/%i zs:%i/%i vs:%i/%i ps:%i/%i tr:%i lit:%i/%i",
			ssc.vbuffer, ssf.vbuffer, ssc.ibuffer, ssf.ibuffer, ssc.declarations, ssf.declarations,
			ssc.renderTarget, ssf.renderTarget, ssc.zStencil, ssf.zStencil,
			ssc.vsh, ssf.vsh, ssc.psh, ssf.psh,
			ssc.transforms, ssc.lighting, ssf.lighting );
		cc.write( buf );

		sprintf( buf, "t:%i/%i rs:%i/%i tss:%i/%i smp:%i/%i const:%i",
			ssc.textures, ssf.textures, ssc.renderStates, ssf.renderStates,
			ssc.textureStages, ssf.textureStages, ssc.samplers, ssf.samplers,
			ssc.vshConst+ssf.pshConst
			);
		cc.write( buf );

		dingus::CD3DDevice::getInstance().getDevice().BeginScene();
		mD3DConsoleCtx->flush();
		dingus::CD3DDevice::getInstance().getDevice().EndScene();
	}

	//
	// poll for close

	if( mApplication->shouldFinish() ) {
		doClose();
	}
	
	return S_OK;
}


/**
 *  Overrrides the main WndProc, so the sample can do custom message handling
 *  (e.g. processing mouse, keyboard, or menu commands).
 */
LRESULT CDingusSystem::msgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	/*
	static bool reloadKeyPressed = false;
	switch( msg ) {
	case WM_KEYDOWN:
		if( wParam == VK_F5 )
			reloadKeyPressed = true;
		break;
	case WM_KEYUP:
		if( wParam == VK_F5 ) {
			if( reloadKeyPressed ) {
				mReloadableManager->reload();
				CEffectParamsNotifier::getInstance().notify();
			}
			reloadKeyPressed = false;
		}
	}
	*/
	if( mApplication && mAppInited ) {
		bool processed = mApplication->msgProc( hWnd, msg, wParam, lParam );
		//if( processed )
		//	return;
	}
	return CD3DApplication::msgProc( hWnd, msg, wParam, lParam );
}


/**
 *  Invalidates device objects.  Paired with activateDeviceObjects().
 */
HRESULT CDingusSystem::passivateDeviceObjects()
{
	APP_CHANNEL << "passivate d3d resources" << endl;
	mFont->passivateDeviceObjects();
	CDeviceResourceManager::getInstance().passivateResource();
	return S_OK;
}

/**
 *  Paired with createDeviceObjects().
 *  Called when the app is exiting, or the device is being changed,
 *  this function deletes any device dependent objects.
 */
HRESULT CDingusSystem::deleteDeviceObjects()
{
	APP_CHANNEL << "delete d3d resources" << endl;
	mFont->deleteDeviceObjects();
	CDeviceResourceManager::getInstance().deleteResource();
	return S_OK;
}


/**
 *  Paired with initialize().
 *  Called before the app exits, this function gives the app the chance
 *  to cleanup after itself.
 */
HRESULT CDingusSystem::shutdown()
{
	APP_CHANNEL << "shutdown app" << endl;
	mApplication->shutdown();

	APP_CHANNEL << "shutdown device resource manager" << endl;
	CDeviceResourceManager::getInstance().clearListeners();
	CDeviceResourceManager::finalize();

	mReloadableManager->clearListeners();
	delete mReloadableManager;
	
	APP_CHANNEL << "destroy contexts" << endl;
	mApplication->destroyContexts();
	APP_CHANNEL << "destroy resource bundles" << endl;
	mApplication->destroyBundles();

	delete mD3DConsoleCtx;
	delete mStdConsoleCtx;
	CConsole::finalize();

	return S_OK;
}



const CD3DEnumeration& CDingusSystem::getD3DEnumeration() const
{
	return mEnumeration;
}

const CD3DSettings& CDingusSystem::getD3DSettings() const
{
	return mSettings;
}

void CDingusSystem::applyD3DSettings( const CD3DSettings& s )
{
	mSettings = s;
	applySettings();
}

HRESULT CDingusSystem::chooseInitialD3DSettings()
{
	APP_CHANNEL << "choose initial d3d settings" << endl;
	assert( mApplication );
	mApplication->initD3DSettingsPref( mSettingsPref );

	return CD3DApplication::chooseInitialD3DSettings();
}
