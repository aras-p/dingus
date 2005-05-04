#include "stdafx.h"

// Demo application.
// Most of stuff is just kept in global variables :)

#include "Demo.h"
#include "DemoResources.h"
#include "Entity.h"
#include "Scene.h"
#include "SceneShared.h"

#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/GfxUtils.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>
#include "PostProcess.h"


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

const char* RMODE_PREFIX[RMCOUNT] = {
	"normal/",
	"reflected/",
	"caster/",
};

CDebugRenderer*	gDebugRenderer;

int			gGlobalCullMode;	// global cull mode
int			gGlobalFillMode;	// global fill mode
SVector4	gScreenFixUVs;		// UV fixes for fullscreen quads
//float		gTimeParam;			// time parameter for effects

bool	gNoPixelShaders = false;

bool	gFinished = false;
bool	gShowStats = false;


// --------------------------------------------------------------------------

CDemo::CDemo()
{
}

bool CDemo::checkDevice( const CD3DDeviceCaps& caps, CD3DDeviceCaps::eVertexProcessing vproc, CD3DEnumErrors& errors )
{
	bool ok = true;
	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 ) {
		if( vproc != CD3DDeviceCaps::VP_SW )
			ok = false;
	}
	// need float textures...
	//if( !caps.hasFloatTextures() ) {
	//	errors.addError( "Floating point rendertargets are required" );
	//	ok = false;
	//}
	
	return ok;
}

bool CDemo::shouldFinish()
{
	return gFinished;
}

bool CDemo::shouldShowStats()
{
	return gShowStats;
}


// --------------------------------------------------------------------------
//  GUI

CUIDialog*		gUIDlg;

CUIStatic*		gUILabFPS;


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}

const int UIHLAB = 14;

static void	gSetupGUI()
{
	gUIDlg->addStatic( 0, "", 3, 480-UIHLAB-1, 500, UIHLAB, false, &gUILabFPS );
	gUIDlg->enableNonUserEvents( true );
}



// --------------------------------------------------------------------------
// Demo variables

// now THAT is a name :)
enum eDemoScene {
	SCENE_MAIN,
	//SCENE_SCROLLER,
	SCENE_INTERACTIVE,
	SCENECOUNT
};

CSceneSharedStuff*	gSceneShared;

// The Scenes (tm)
CSceneMain*			gSceneMain;
CSceneInteractive*	gSceneInt;

int			gCurScene = SCENE_MAIN;


// normally system timer, but controllable for debugging
CTimer			gDemoTimer; 



float		gMouseX; // from -1 to 1
float		gMouseY; // from -1 to 1
SVector3	gMouseRay;



SMatrix4x4		gCameraViewProjMatrix;
SMatrix4x4		gViewTexProjMatrix;
CPostProcess*	gPPReflBlur;


//std::vector<int>	gMousePieces[CFACE_COUNT];

int		gWallVertCount, gWallTriCount;



CRenderableMesh*	gQuadGaussX;
CRenderableMesh*	gQuadGaussY;
CRenderableMesh*	gQuadBlur;



struct SShadowLight {
public:
	void initialize( const SVector3& pos, const SVector3& lookAt ) {
		SMatrix4x4 viewMat;
		D3DXMatrixLookAtLH( &viewMat, &pos, &lookAt, &SVector3(1,0,0) );
		D3DXMatrixInverse( &camera.mWorldMat, NULL, &viewMat );
		camera.setProjectionParams( D3DX_PI/5, 1.0f, pos.y*0.1f, pos.y*2.0f );
		viewProj = viewMat * camera.getProjectionMatrix();
	}

public:
	CCameraEntity	camera;
	SMatrix4x4		viewProj;
};



// --------------------------------------------------------------------------
// Shadow mapping


const SVector3 LIGHT_POS = SVector3( ROOM_MID.x, ROOM_MAX.y*1.5f, ROOM_MID.z );

SShadowLight	gSLight;
SMatrix4x4		gSLightVP;
SMatrix4x4		gSShadowProj;
SVector3		gSLightPos = LIGHT_POS;


void gShadowRender( CScene& scene )
{
	const SMatrix4x4* lightTargetMat = scene.getLightTargetMatrix();
	if( !lightTargetMat )
		return;

	CD3DDevice& dx = CD3DDevice::getInstance();

	// target the light where needed
	gSLight.initialize( LIGHT_POS, lightTargetMat->getOrigin() );

	// Leave one texel padding...
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 1; vp.Height = vp.Width = SZ_SHADOWMAP-2;
	vp.MinZ = 0.0f; vp.MaxZ = 1.0f;

	gSLight.camera.setOntoRenderContext();

	gSLightVP = gSLight.viewProj;
	gSLightPos = gSLight.camera.mWorldMat.getOrigin();
	gfx::textureProjectionWorld( gSLightVP, SZ_SHADOWMAP, SZ_SHADOWMAP, gSShadowProj );

	// render shadow map
	dx.setZStencil( RGET_S_SURF(RT_SHADOWZ) );
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWMAP) );
	//dx.clearTargets( true, true, false, 0xFFffffff, 0.0f ); // min based dilation
	dx.clearTargets( true, true, false, 0xFFff00ff, 0.0f ); // gauss

	dx.getDevice().SetViewport( &vp );

	dx.sceneBegin();
	scene.render( RM_CASTERSIMPLE );
	G_RENDERCTX->applyGlobalEffect();

	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );
	G_RENDERCTX->perform();
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	dx.sceneEnd();

	// process shadowmap
	dx.setZStencil( NULL );

	// gaussX shadowmap -> shadowblur
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWBLUR) );
	dx.sceneBegin();
	G_RENDERCTX->attach( *gQuadGaussX );
	G_RENDERCTX->perform();
	dx.sceneEnd();
	// gaussY shadowblur -> shadowmap
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWMAP) );
	dx.sceneBegin();
	G_RENDERCTX->attach( *gQuadGaussY );
	G_RENDERCTX->perform();
	dx.sceneEnd();
	// blur shadowmap -> shadowblur
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWBLUR) );
	dx.clearTargets( true, false, false, 0xFFffffff );
	dx.getDevice().SetViewport( &vp );
	dx.sceneBegin();
	G_RENDERCTX->attach( *gQuadBlur );
	G_RENDERCTX->perform();
	dx.sceneEnd();
}

// --------------------------------------------------------------------------
// reflective walls

void gRenderWallReflections( CScene& scene )
{
	SVector3 planePos[CFACE_COUNT] = {
		SVector3(ROOM_MAX.x,0,0), SVector3(ROOM_MIN.x,0,0),
		SVector3(0,ROOM_MAX.y,0), SVector3(0,ROOM_MIN.y,0),
		SVector3(0,0,ROOM_MAX.z), SVector3(0,0,ROOM_MIN.z),
	};
	SVector3 planeNrm[CFACE_COUNT] = {
		SVector3(-1,0,0), SVector3(1,0,0),
		SVector3(0,-1,0), SVector3(0,1,0),
		SVector3(0,0,-1), SVector3(0,0,1),
	};

	int oldCull = gGlobalCullMode;
	gGlobalCullMode = D3DCULL_CCW;


	CCameraEntity& wallCam = gSceneShared->getWallCamera();
	const CCameraEntity& camera = scene.getCamera();

	
	for( int currWall = 0; currWall < CFACE_COUNT; ++currWall ) {
		//gWallMeshes[currWall]->updateWVPMatrices();
		//if( gWallMeshes[currWall]->frustumCull(gCameraViewProjMatrix) )
		//	continue;

		SPlane reflPlane( planePos[currWall] + planeNrm[currWall]*0.05f, planeNrm[currWall] );
		SMatrix4x4 reflectMat;
		D3DXMatrixReflect( &reflectMat, &reflPlane );
		
		wallCam.mWorldMat = camera.mWorldMat * reflectMat;
		wallCam.setProjFrom( camera );
		wallCam.setOntoRenderContext();

		CD3DDevice& dx = CD3DDevice::getInstance();
		dx.setRenderTarget( RGET_S_SURF(RT_REFLRT) );
		dx.setZStencil( RGET_S_SURF(RT_REFLZ) );
		dx.clearTargets( true, true, false, 0xFF000020, 1.0f );
		dx.sceneBegin();
		G_RENDERCTX->applyGlobalEffect();
		scene.render( RM_REFLECTED );
		G_RENDERCTX->perform();

		dx.sceneEnd();

		// Now reflected stuff is in RT_REFLRT surface. Blur it!
		gPPReflBlur->downsampleRT( *RGET_S_SURF(RT_REFLRT)->getObject() );
		dx.getStateManager().SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		gPPReflBlur->pingPongBlur( 2 );
		dx.getStateManager().SetRenderState( D3DRS_CULLMODE, gGlobalCullMode );

		// Now blurred stuff is in RT_REFL_TMP1. Copy it to our needed texture.
		IDirect3DSurface9* surf;
		RGET_S_TEX(WALL_TEXS[currWall])->getObject()->GetSurfaceLevel( 0, &surf );
		dx.getDevice().StretchRect( RGET_S_SURF(RT_REFL_TMP1)->getObject(), NULL, surf, NULL, dx.getCaps().getStretchFilter() );
		surf->Release();
	}

	gGlobalCullMode = oldCull;
}

// --------------------------------------------------------------------------
// Initialization


void CDemo::initialize( IDingusAppContext& appContext )
{
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	gNoPixelShaders = (dx.getCaps().getPShaderVersion() < CD3DDeviceCaps::PS_1_1);

	// --------------------------------
	// render targets

	// shadow maps
	if( !gNoPixelShaders ) {
		ITextureCreator* shadowT = new CFixedTextureCreator(
			SZ_SHADOWMAP, SZ_SHADOWMAP, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
		ITextureCreator* shadowTMip = new CFixedTextureCreator(
			SZ_SHADOWMAP, SZ_SHADOWMAP, 0, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
		stb.registerTexture( RT_SHADOWMAP, *shadowT );
		stb.registerTexture( RT_SHADOWBLUR, *shadowTMip );
		ssb.registerSurface( RT_SHADOWMAP, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWMAP),0)) );
		ssb.registerSurface( RT_SHADOWBLUR, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWBLUR),0)) );
		ssb.registerSurface( RT_SHADOWZ, *(new CFixedSurfaceCreator(SZ_SHADOWMAP,SZ_SHADOWMAP,true,D3DFMT_D16)) );

		G_RENDERCTX->getGlobalParams().addTexture( "tShadow", *RGET_S_TEX(RT_SHADOWBLUR) );
	}
	
	// reflections
	if( !gNoPixelShaders ) {
		ISurfaceCreator* rtcreatReflRT = new CScreenBasedSurfaceCreator(
			SZ_REFLRT_REL, SZ_REFLRT_REL, false, D3DFMT_A8R8G8B8, false );
		ISurfaceCreator* rtcreatReflZ = new CScreenBasedSurfaceCreator(
			SZ_REFLRT_REL, SZ_REFLRT_REL, true, D3DFMT_D16, false );
		ITextureCreator* rtcreatReflBlur = new CScreenBasedTextureCreator(
			SZ_REFLBLUR_REL, SZ_REFLBLUR_REL, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
		stb.registerTexture( RT_REFL_PX, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_NX, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_PY, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_NY, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_PZ, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_NZ, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_TMP1, *rtcreatReflBlur );
		stb.registerTexture( RT_REFL_TMP2, *rtcreatReflBlur );
		ssb.registerSurface( RT_REFLRT, *rtcreatReflRT );
		ssb.registerSurface( RT_REFLZ, *rtcreatReflZ );
		ssb.registerSurface( RT_REFL_TMP1, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_REFL_TMP1),0)) );
		ssb.registerSurface( RT_REFL_TMP2, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_REFL_TMP2),0)) );
	}

	// --------------------------------
	// common params

	gGlobalCullMode = D3DCULL_CW;
	gGlobalFillMode = D3DFILL_SOLID;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addIntRef( "iFill", &gGlobalFillMode );
	//G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );

	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mViewTexProj", gViewTexProjMatrix );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mShadowProj", gSShadowProj );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mLightViewProj", gSLightVP );
	G_RENDERCTX->getGlobalParams().addVector3Ref( "vLightPos", gSLightPos );

	gDebugRenderer = new CDebugRenderer( *G_RENDERCTX, *RGET_FX("debug") );

	// --------------------------------
	// GUI

	gUIDlg = new CUIDialog();
	//gUIDlg->enableKeyboardInput( true );
	gUIDlg->setCallback( gUICallback );
	gUIDlg->setFont( 1, "Arial", 22, 50 );

	gSetupGUI();

	// --------------------------------
	// scenes

	gSceneShared = new CSceneSharedStuff();
	gSceneMain = new CSceneMain( gSceneShared );
	gSceneInt = new CSceneInteractive( gSceneShared );


	if( !gNoPixelShaders ) {
		// post processes
		gPPReflBlur = new CPostProcess( RT_REFL_TMP1, RT_REFL_TMP2 );
		// gauss X shadowmap -> shadowblur
		gQuadGaussX = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
		gQuadGaussX->getParams().setEffect( *RGET_FX("filterGaussX") );
		gQuadGaussX->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOWMAP) );
		// gauss Y shadowblur -> shadowmap
		gQuadGaussY = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
		gQuadGaussY->getParams().setEffect( *RGET_FX("filterGaussY") );
		gQuadGaussY->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOWBLUR) );
		// blur shadowmap -> shadowblur
		gQuadBlur = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
		gQuadBlur->getParams().setEffect( *RGET_FX("filterPoisson") );
		gQuadBlur->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOWMAP) );
	}
}



// --------------------------------------------------------------------------
// Perform code (main loop)



/*
static void gFetchMousePieces( bool fractureOut )
{
	int i;

	gMouseRay = gCamera.getWorldRay( gMouseX, gMouseY );
	const SVector3& eyePos = gCamera.mWorldMat.getOrigin();
	SLine3 mouseRay;
	mouseRay.pos = eyePos;
	mouseRay.vec = gMouseRay;

	// intersect mouse with walls
	float minWallT = 1.0e6f;
	for( i = 0; i < CFACE_COUNT; ++i ) {
		float t;
		bool ok = gWalls[i]->intersectRay( mouseRay, t );
		if( ok && t < minWallT )
			minWallT = t;
	}
	SVector3 mousePos = eyePos + gMouseRay * minWallT;
	const float MOUSE_RADIUS = 0.6f;

	double t = CSystemTimer::getInstance().getTimeS();
	if( fractureOut ) {
		CConsole::CON_WARNING << mousePos << endl;
	}
	for( i = 0; i < CFACE_COUNT; ++i ) {
		gWalls[i]->fracturePiecesInSphere( t, fractureOut, mousePos, MOUSE_RADIUS, gMousePieces[i] );
	}
}
*/

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( !gUIDlg )
		return false;

	bool done = false;

	done = gUIDlg->msgProc( hwnd, msg, wparam, lparam );
	if( done )
		return true;
	
	// track mouse...
	if( msg == WM_LBUTTONDOWN ) {
		/*
		gFetchMousePieces( true );
		for( int i = 0; i < CFACE_COUNT; ++i ) {
			int n = gMousePieces[i].size();
			for( int j = 0; j < n; ++j ) {
				wall_phys::spawnPiece( i, gMousePieces[i][j] );
			}
		}
		*/
	}
	if( msg == WM_MOUSEMOVE ) {
		CD3DDevice& dx = CD3DDevice::getInstance();
		gMouseX = (float(LOWORD(lparam)) / dx.getBackBufferWidth()) * 2 - 1;
		gMouseY = (float(HIWORD(lparam)) / dx.getBackBufferHeight()) * 2 - 1;
	}
	
	return false;
}

static float	gInputTargetMoveSpeed = 0.0f;
static float	gInputTargetRotpeed = 0.0f;
static bool		gInputAttack = false;


void CDemo::onInputEvent( const CInputEvent& event )
{
	time_value animTime = gDemoTimer.getTime();
	static bool shiftPressed = false;
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	if( event.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)event;
		switch( ke.getKeyCode() ) {
		case DIK_LSHIFT:
		case DIK_RSHIFT:
			shiftPressed = (ke.getMode() != CKeyEvent::KEY_RELEASED);
			break;

		case DIK_9:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gShowStats = !gShowStats;
			break;
		case DIK_RETURN:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				++gCurScene;
				gCurScene %= SCENECOUNT;
			}
			break;
		case DIK_SPACE:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				if( gCurScene == SCENE_INTERACTIVE ) {
					gInputAttack = true;
				}
			}
			break;
		case DIK_LEFT:
			if( gCurScene == SCENE_INTERACTIVE )
				gInputTargetRotpeed -= 3.0f;
			break;
		case DIK_RIGHT:
			if( gCurScene == SCENE_INTERACTIVE )
				gInputTargetRotpeed += 3.0f;
			break;
		case DIK_UP:
			if( gCurScene == SCENE_INTERACTIVE ) {
				gInputTargetMoveSpeed += shiftPressed ? 0.8f : 0.2f;
			}
			break;
		case DIK_DOWN:
			if( gCurScene == SCENE_INTERACTIVE ) {
				gInputTargetMoveSpeed -= 0.2f;
			}
			break;

		case DIK_1:
			gDemoTimer.update( time_value::fromsec(-dt*10) );
			break;
		case DIK_2:
			gDemoTimer.update( time_value::fromsec(-dt*2) );
			break;
		case DIK_3:
			gDemoTimer.update( time_value::fromsec(-dt) );
			break;
		case DIK_4:
			gDemoTimer.update( time_value::fromsec(-dt*0.8f) );
			break;
		case DIK_5:
			gDemoTimer.update( time_value::fromsec(dt*4) );
			break;
		case DIK_6:
			gDemoTimer.update( time_value::fromsec(dt*20) );
			break;
		case DIK_7:
			gDemoTimer.update( time_value::fromsec(dt*100) );
			break;
		/*
		case DIK_A:
			if( gCameraFollow ) {
				gCameraDist += dt*4;
				gCameraDist = clamp( gCameraDist, 0.5f, 7.0f );
			} else {
				gCamera.mWorldMat.getOrigin() += gCamera.mWorldMat.getAxisZ() * dt * 3;
			}
			break;
		case DIK_Z:
			if( gCameraFollow ) {
				gCameraDist -= dt*4;
				gCameraDist = clamp( gCameraDist, 0.5f, 7.0f );
			} else {
				gCamera.mWorldMat.getOrigin() -= gCamera.mWorldMat.getAxisZ() * dt * 3;
			}
			break;
		*/
		}
	}
}

static char gMoveDebugBuf[1000];

void CDemo::onInputStage()
{
	if( gCurScene == SCENE_INTERACTIVE ) {
		time_value animTime = gDemoTimer.getTime();
		gSceneInt->processInput( gInputTargetMoveSpeed, gInputTargetRotpeed, gInputAttack, animTime );
	}


	gInputAttack = false;
	gInputTargetRotpeed = 0.0f;
	gInputTargetMoveSpeed = 0.0f;
}




//  Main loop code.
void CDemo::perform()
{
	char buf[1000];

	CDynamicVBManager::getInstance().discard();
	CDynamicIBManager::getInstance().discard();

	G_INPUTCTX->perform();

	// timing
	double dt;
	static bool firstPerform = true;
	if( firstPerform ) {
		dt = 0.0;
		firstPerform = false;
	} else {
		dt = CSystemTimer::getInstance().getDeltaTimeS();
		gDemoTimer.update( CSystemTimer::getInstance().getDeltaTime() );
	}
	time_value demoTime = gDemoTimer.getTime();

	// figure out current scene
	CScene* curScene = NULL;
	switch( gCurScene ) {
	case SCENE_MAIN:		curScene = gSceneMain; break;
	case SCENE_INTERACTIVE:	curScene = gSceneInt; break;
	}
	assert( curScene );

	// update scene
	curScene->update( demoTime, dt );


	gWallVertCount = gWallTriCount = 0;

	CD3DDevice& dx = CD3DDevice::getInstance();
	
	gScreenFixUVs.set( 0.5f/dx.getBackBufferWidth(), 0.5f/dx.getBackBufferHeight(), 0.0f, 0.0f );
	
	
	// FPS
	/*
	static float maxMsColl = 0;
	static float maxMsPhys = 0;
	static const int MAGIC_COUNT = 1550;
	const wall_phys::SStats& stats = wall_phys::getStats();

	if( stats.msColl > maxMsColl && stats.pieceCount < MAGIC_COUNT )
		maxMsColl = stats.msColl;
	if( stats.msPhys > maxMsPhys && stats.pieceCount < MAGIC_COUNT )
		maxMsPhys = stats.msPhys;
	sprintf( buf, "fps=%.1f  frame=%.1f  phys: c=%.1f (%.1f) p=%.1f (%.1f) u=%.1f ms  pieces: %i",
		dx.getStats().getFPS(),
		gCurrAnimFrame,
		stats.msColl,
		maxMsColl,
		stats.msPhys,
		maxMsPhys,
		stats.msUpdate,
		stats.pieceCount
	);
	*/
	sprintf( buf, "fps=%.1f  time=%.1f (%.1f)",
		dx.getStats().getFPS(),
		demoTime.tosec(),
		demoTime.tosec()*ANIM_FPS
	);
	gUILabFPS->setText( buf );

	//gFetchMousePieces( false );

	
	if( !gNoPixelShaders ) {
		// render shadow map
		gShadowRender( *curScene );
		// render wall reflections
		gRenderWallReflections( *curScene );
	}

	curScene->getCamera().setOntoRenderContext();
	gCameraViewProjMatrix = G_RENDERCTX->getCamera().getViewProjMatrix();
	gfx::textureProjectionWorld( gCameraViewProjMatrix, 1000.0f, 1000.0f, gViewTexProjMatrix );

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFF000080, 1.0f, 0L );

	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();
	curScene->render( RM_NORMAL );
	G_RENDERCTX->perform();
	
	// render GUI
	gUIDlg->onRender( dt );
	dx.sceneEnd();


	/*
	static int maxVerts = 0;
	if( gWallVertCount + stats.vertexCount > maxVerts )
		maxVerts = gWallVertCount + stats.vertexCount;
	static int maxTris = 0;
	if( gWallTriCount + stats.triCount > maxTris )
		maxTris = gWallTriCount + stats.triCount;

	if( gShowStats ) {
		CConsole::getChannel("system") << "wall geom: verts=" << gWallVertCount << " tris=" << gWallTriCount << endl;
		CConsole::getChannel("system") << "phys geom: verts=" << stats.vertexCount << " tris=" << stats.triCount << endl;
		CConsole::getChannel("system") << "max: verts=" << maxVerts << " (" << int(maxVerts*sizeof(SVertexXyzDiffuse)) << ")  tris=" << maxTris << " (" << maxTris*2*3 << ")" << endl;
	}
	*/

	if( gCurScene == SCENE_MAIN && gSceneMain->isEnded() ) { 
		gFinished = true;
	}
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	delete gDebugRenderer;

	safeDelete( gUIDlg );
	safeDelete( gPPReflBlur );
	delete gSceneMain;
	delete gSceneInt;
	delete gSceneShared;
}
