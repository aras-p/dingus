#include "stdafx.h"

// Demo application.
// Most of stuff is just kept in global variables :)

#include "Demo.h"
#include "DemoResources.h"
#include "Entity.h"
#include "SceneMain.h"
#include "SceneScroller.h"
#include "SceneInteractive.h"
#include "SceneShared.h"
#include "Tweaker.h"
#include "../system/MusicPlayer.h"

#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/GfxUtils.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>
#include "PostProcess.h"
#include <ctime>


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

const char* RMODE_PREFIX[RMCOUNT] = {
	"normal/",
	"reflected/",
	"caster/",
	"castersimple/",
};

CDebugRenderer*	gDebugRenderer;

int			gGlobalCullMode;	// global cull mode
SVector4	gScreenFixUVs;		// UV fixes for fullscreen quads
float		gTimeParam;			// time parameter for effects

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
	if( caps.getPShaderVersion() < CD3DDeviceCaps::PS_2_0 ) {
		errors.addError( "Pixel shaders 2.0 required" );
		ok = false;
	}
	
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

CUIImage*		gUIImgLoading;
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
	SCENE_SCROLLER,
	SCENE_INTERACTIVE,
	SCENECOUNT
};

CSceneSharedStuff*	gSceneShared;

// The Scenes (tm)
CSceneMain*			gSceneMain;
CSceneInteractive*	gSceneInt;
CSceneScroller*		gSceneScroller;


int			gCurScene = SCENE_MAIN;


// normally music/system timer, but controllable for debugging
CTimer			gDemoTimer;
bool			gPaused = false;

float		gCharTimeBlend;
SVector4	gDOFParams;
int			gDOFPasses1 = 1;
int			gDOFPasses2 = 2;
float		gDOFSpreadMult = 1.0f;

void	gSetDOFBlurBias( float blur )
{
	gDOFPasses1 = 1;
	gDOFPasses2 = 2;
	gDOFSpreadMult = 1.0f + blur*3.0f;
	if( blur > 0.25f ) {
		gDOFSpreadMult *= 3.0f / 4.0f;
		++gDOFPasses2;
	}
	if( blur > 0.5f ) {
		gDOFSpreadMult *= 4.0f / 5.0f;
		++gDOFPasses2;
	}
	if( blur > 0.75f ) {
		gDOFSpreadMult *= 5.0f / 6.0f;
		++gDOFPasses2;
	}
}



float		gMouseX; // from -1 to 1
float		gMouseY; // from -1 to 1
SVector3	gMouseRay;



SMatrix4x4		gCameraViewProjMatrix;
SMatrix4x4		gViewTexProjMatrix;
SPlane			gReflPlane;
CPostProcess*	gPPReflBlur;
CPostProcess*	gPPDofBlur;


int		gWallVertCount, gWallTriCount;



CRenderableMesh*	gQuadGaussX;
CRenderableMesh*	gQuadGaussY;
CRenderableMesh*	gQuadBlur;
CRenderableMesh*	gQuadDOF;



// --------------------------------------------------------------------------
// Shadow mapping


struct SShadowLight {
public:
	void initialize( const SVector3& pos, const SVector3& lookAt, float fov ) {
		SMatrix4x4 viewMat;
		D3DXMatrixLookAtLH( &viewMat, &pos, &lookAt, &SVector3(1,0.01f,0.01f) );
		D3DXMatrixInverse( &camera.mWorldMat, NULL, &viewMat );
		camera.setProjectionParams( fov, 1.0f, pos.y*0.1f, pos.y*2.0f );
		viewProj = viewMat * camera.getProjectionMatrix();
	}

public:
	CCameraEntity	camera;
	SMatrix4x4		viewProj;
};



// Main, tightly targeted at character. Soft shadows.
SShadowLight	gSLight;
SMatrix4x4		gSShadowProj;

// Secondary, covers floor, misc. pieces render into it. Standard proj. shadows.
SShadowLight	gSLight2;
SMatrix4x4		gSShadowProj2;


void gShadowRender( CScene& scene )
{
	const SMatrix4x4* lightTargetMat = scene.getLightTargetMatrix();
	if( !lightTargetMat )
		return;

	CD3DDevice& dx = CD3DDevice::getInstance();

	// target the light where needed
	gSLight.initialize( LIGHT_POS_1, lightTargetMat->getOrigin(), D3DX_PI/5 );

	// Leave one texel padding...
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 1; vp.Height = vp.Width = SZ_SHADOWMAP-2;
	vp.MinZ = 0.0f; vp.MaxZ = 1.0f;

	gSLight.camera.setOntoRenderContext();

	gfx::textureProjectionWorld( gSLight.viewProj, SZ_SHADOWMAP, SZ_SHADOWMAP, gSShadowProj );

	// render soft shadow map

	dx.setZStencil( RGET_S_SURF(RT_SHADOWZ) );
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWMAP) );
	dx.clearTargets( true, true, false, 0xFFff00ff, 0.0f ); // gauss

	dx.getDevice().SetViewport( &vp );

	dx.sceneBegin();
	scene.render( RM_CASTER );
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


void gShadowRender2( CScene& scene )
{

	CD3DDevice& dx = CD3DDevice::getInstance();

	// target the light where needed
	const SVector3 LIGHT_TARGET_POS( ROOM_MID.x, ROOM_MIN.y, ROOM_MID.z );
	gSLight2.initialize( LIGHT_POS_1, LIGHT_TARGET_POS, D3DX_PI*0.45f );

	// Leave one texel padding...
	//D3DVIEWPORT9 vp;
	//vp.X = vp.Y = 1; vp.Height = vp.Width = SZ_SHADOWMAP2-2;
	//vp.MinZ = 0.0f; vp.MaxZ = 1.0f;

	gSLight2.camera.setOntoRenderContext();

	gfx::textureProjectionWorld( gSLight2.viewProj, SZ_SHADOWMAP2_SM, SZ_SHADOWMAP2_SM, gSShadowProj2 );

	// render
	dx.setZStencil( RGET_S_SURF(RT_SHADOWZ2) );
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWMAP2_BIG) );
	dx.clearTargets( true, true, false, 0xFFffffff, 1.0f );

	//dx.getDevice().SetViewport( &vp );

	dx.sceneBegin();
	scene.render( RM_CASTERSIMPLE );
	G_RENDERCTX->applyGlobalEffect();
	G_RENDERCTX->perform();
	dx.sceneEnd();

	dx.getDevice().StretchRect(
		RGET_S_SURF(RT_SHADOWMAP2_BIG)->getObject(), NULL,
		RGET_S_SURF(RT_SHADOWMAP2_SM)->getObject(), NULL,
		dx.getCaps().getStretchFilter() );
}


// --------------------------------------------------------------------------
// reflective walls

void gRenderWallReflections( CScene& scene )
{
	if( !scene.needsReflections() )
		return;

	static SVector3 planePos[CFACE_COUNT] = {
		SVector3(ROOM_MAX.x,0,0), SVector3(ROOM_MIN.x,0,0),
		SVector3(0,ROOM_MAX.y,0), SVector3(0,ROOM_MIN.y,0),
		SVector3(0,0,ROOM_MAX.z), SVector3(0,0,ROOM_MIN.z),
	};
	static SVector3 planeNrm[CFACE_COUNT] = {
		SVector3(-1,0,0), SVector3(1,0,0),
		SVector3(0,-1,0), SVector3(0,1,0),
		SVector3(0,0,-1), SVector3(0,0,1),
	};

	int oldCull = gGlobalCullMode;
	gGlobalCullMode = D3DCULL_CCW;


	CCameraEntity& wallCam = gSceneShared->getWallCamera();
	const CCameraEntity& camera = scene.getCamera();

	
	for( int currWall = 0; currWall < CFACE_COUNT; ++currWall ) {
		if( gSceneShared->cullWall( currWall, gCameraViewProjMatrix ) )
			continue;

		gReflPlane = SPlane( planePos[currWall] + planeNrm[currWall]*0.05f, planeNrm[currWall] );
		SMatrix4x4 reflectMat;
		D3DXMatrixReflect( &reflectMat, &gReflPlane );
		
		wallCam.mWorldMat = camera.mWorldMat * reflectMat;
		wallCam.setProjFrom( camera );
		wallCam.setOntoRenderContext();

		CD3DDevice& dx = CD3DDevice::getInstance();
		dx.setRenderTarget( RGET_S_SURF(RT_HALFRT) );
		dx.setZStencil( RGET_S_SURF(RT_HALFZ) );
		dx.clearTargets( true, true, false, 0xFFffffff, 1.0f );
		dx.sceneBegin();
		G_RENDERCTX->applyGlobalEffect();
		scene.render( RM_REFLECTED );
		G_RENDERCTX->perform();

		dx.sceneEnd();

		// Now reflected stuff is in RT_REFLRT surface. Blur it!
		gPPReflBlur->downsampleRT( *RGET_S_SURF(RT_HALFRT)->getObject() );
		dx.getStateManager().SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		gPPReflBlur->pingPongBlur( 2 );
		dx.getStateManager().SetRenderState( D3DRS_CULLMODE, gGlobalCullMode );

		// Now blurred stuff is in RT_REFL_TMP1. Copy it to our needed texture.
		IDirect3DSurface9* surf;
		RGET_S_TEX(WALL_TEXS[currWall])->getObject()->GetSurfaceLevel( 0, &surf );
		//dx.getDevice().StretchRect( RGET_S_SURF(RT_REFLRT)->getObject(), NULL, surf, NULL, dx.getCaps().getStretchFilter() );
		dx.getDevice().StretchRect( RGET_S_SURF(RT_QUAD_TMP1)->getObject(), NULL, surf, NULL, dx.getCaps().getStretchFilter() );
		surf->Release();
	}

	gGlobalCullMode = oldCull;
}


// --------------------------------------------------------------------------
// DOF

static void gRenderDOF()
{
	const int BLUR_PASSES_1 = gDOFPasses1;
	const int BLUR_PASSES_2 = gDOFPasses2;
	const int BLUR_PASSES = BLUR_PASSES_1 + BLUR_PASSES_2;

	CD3DDevice& dx = CD3DDevice::getInstance();

	// copy backbuffer to texture
	dx.getDevice().StretchRect(
		dx.getBackBuffer(), NULL,
		RGET_S_SURF(RT_FULLSCREEN)->getObject(), NULL,
		D3DTEXF_NONE
	);

	// blur 1st phase
	gPPDofBlur->downsampleRT( *RGET_S_SURF(RT_FULLSCREEN)->getObject() );
	dx.getStateManager().SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	gPPDofBlur->pingPongBlur( BLUR_PASSES_1, 0, gDOFSpreadMult );
	
	dx.getDevice().StretchRect(
		RGET_S_SURF( !(BLUR_PASSES_1&1) ? RT_HALF_TMP1 : RT_HALF_TMP2 )->getObject(), NULL,
		RGET_S_SURF(RT_DOF_1)->getObject(), NULL,
		D3DTEXF_NONE
	);

	// blur 2nd phase
	gPPDofBlur->pingPongBlur( BLUR_PASSES_2, BLUR_PASSES_1, gDOFSpreadMult );
	dx.getDevice().StretchRect(
		RGET_S_SURF( !(BLUR_PASSES&1) ? RT_HALF_TMP1 : RT_HALF_TMP2 )->getObject(), NULL,
		RGET_S_SURF(RT_DOF_2)->getObject(), NULL,
		D3DTEXF_NONE
	);

	// composite
	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.sceneBegin();
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadDOF );
	G_RENDERCTX->directEnd();
	dx.sceneEnd();

	dx.getStateManager().SetRenderState( D3DRS_CULLMODE, gGlobalCullMode );
}


// --------------------------------------------------------------------------
// Initialization


void CALLBACK gUIRenderCallback( CUIDialog& dlg )
{
	// figure out current scene
	CScene* curScene = NULL;
	switch( gCurScene ) {
	case SCENE_MAIN:		curScene = gSceneMain; break;
	case SCENE_SCROLLER:	curScene = gSceneScroller; break;
	case SCENE_INTERACTIVE:	curScene = gSceneInt; break;
	}
	assert( curScene );
	
	curScene->renderUI( dlg );
}


void CDemo::initialize( IDingusAppContext& appContext )
{
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();

	// --------------------------------
	// render targets

	tweaker::init();

	// shadow maps
	ITextureCreator* shadowT = new CFixedTextureCreator(
		SZ_SHADOWMAP, SZ_SHADOWMAP, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
	ITextureCreator* shadowTMip = new CFixedTextureCreator(
		SZ_SHADOWMAP, SZ_SHADOWMAP, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
	stb.registerTexture( RT_SHADOWMAP, *shadowT );
	stb.registerTexture( RT_SHADOWBLUR, *shadowTMip );
	ssb.registerSurface( RT_SHADOWMAP, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWMAP),0)) );
	ssb.registerSurface( RT_SHADOWBLUR, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWBLUR),0)) );
	ssb.registerSurface( RT_SHADOWZ, *(new CFixedSurfaceCreator(SZ_SHADOWMAP,SZ_SHADOWMAP,true,D3DFMT_D16)) );

	stb.registerTexture( RT_SHADOWMAP2_SM, *new CFixedTextureCreator(
		SZ_SHADOWMAP2_SM, SZ_SHADOWMAP2_SM, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT ) );
	ssb.registerSurface( RT_SHADOWMAP2_SM, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWMAP2_SM),0)) );

	ssb.registerSurface( RT_SHADOWMAP2_BIG, *(new CFixedSurfaceCreator(SZ_SHADOWMAP2_BIG,SZ_SHADOWMAP2_BIG,false,D3DFMT_R5G6B5)) );
	ssb.registerSurface( RT_SHADOWZ2, *(new CFixedSurfaceCreator(SZ_SHADOWMAP2_BIG,SZ_SHADOWMAP2_BIG,true,D3DFMT_D16)) );

	// misc
	stb.registerTexture( RT_FULLSCREEN, *new CScreenBasedTextureCreator(1.0f,1.0f,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT) );
	ssb.registerSurface( RT_FULLSCREEN, *new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_FULLSCREEN),0) );

	// reflections / DOF
	ITextureCreator* rtcreatDofRT = new CScreenBasedTextureCreator(
		SZ_HALF_REL, SZ_HALF_REL, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );

	ISurfaceCreator* rtcreatReflRT = new CScreenBasedSurfaceCreator(
		SZ_HALF_REL, SZ_HALF_REL, false, D3DFMT_A8R8G8B8, false );
	ISurfaceCreator* rtcreatReflZ = new CScreenBasedSurfaceCreator(
		SZ_HALF_REL, SZ_HALF_REL, true, D3DFMT_D16, false );
	
	ITextureCreator* rtcreat2th = new CScreenBasedTextureCreator(
		SZ_HALF_REL, SZ_HALF_REL, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );
	ITextureCreator* rtcreat4th = new CScreenBasedTextureCreator(
		SZ_QUAT_REL, SZ_QUAT_REL, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT );

	stb.registerTexture( RT_REFL_PX, *rtcreat4th );
	stb.registerTexture( RT_REFL_NX, *rtcreat4th );
	stb.registerTexture( RT_REFL_PY, *rtcreat4th );
	stb.registerTexture( RT_REFL_NY, *rtcreat4th );
	stb.registerTexture( RT_REFL_PZ, *rtcreat4th );
	stb.registerTexture( RT_REFL_NZ, *rtcreat4th );

	stb.registerTexture( RT_HALF_TMP1, *rtcreat2th );
	stb.registerTexture( RT_HALF_TMP2, *rtcreat2th );
	stb.registerTexture( RT_QUAD_TMP1, *rtcreat4th );
	stb.registerTexture( RT_QUAD_TMP2, *rtcreat4th );
	ssb.registerSurface( RT_HALF_TMP1, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_HALF_TMP1),0)) );
	ssb.registerSurface( RT_HALF_TMP2, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_HALF_TMP2),0)) );
	ssb.registerSurface( RT_QUAD_TMP1, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_QUAD_TMP1),0)) );
	ssb.registerSurface( RT_QUAD_TMP2, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_QUAD_TMP2),0)) );

	stb.registerTexture( RT_DOF_1, *rtcreatDofRT );
	stb.registerTexture( RT_DOF_2, *rtcreatDofRT );
	ssb.registerSurface( RT_DOF_1, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_DOF_1),0)) );
	ssb.registerSurface( RT_DOF_2, *(new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_DOF_2),0)) );

	ssb.registerSurface( RT_HALFRT, *rtcreatReflRT );
	ssb.registerSurface( RT_HALFZ, *rtcreatReflZ );

	// --------------------------------
	// common params

	gGlobalCullMode = D3DCULL_CW;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );

	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mViewTexProj", gViewTexProjMatrix );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mShadowProj", gSShadowProj );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mShadowProj2", gSShadowProj2 );
	G_RENDERCTX->getGlobalParams().addVector4Ref( "vScreenFixUV", gScreenFixUVs );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fCharTimeBlend", &gCharTimeBlend );
	G_RENDERCTX->getGlobalParams().addVector4Ref( "vDOF", gDOFParams );

	gDebugRenderer = new CDebugRenderer( *G_RENDERCTX, *RGET_FX("debug") );

	// --------------------------------
	// GUI

	gUIDlg = new CUIDialog();
	//gUIDlg->enableKeyboardInput( true );
	gUIDlg->setCallback( gUICallback );
	gUIDlg->setFont( 1, "Verdana", 24, FW_BOLD );
	gUIDlg->setFont( 2, "Verdana", 20, FW_NORMAL );
	gUIDlg->setFont( 3, "Verdana", 16, FW_NORMAL );
	gUIDlg->setFont( 4, "Verdana", 34, FW_BOLD );
	gUIDlg->setFont( 5, "Verdana", 14, FW_BOLD );
	gUIDlg->setFont( 6, "Verdana", 28, FW_BOLD );

	gUIDlg->addImage( 0, 0, 0, GUI_X, GUI_Y, *RGET_TEX("Title"), 0, 0, 1024, 768, &gUIImgLoading );
	gUIImgLoading->getElement(0)->colorTexture.current = 0xFFffffff;

	// bulk of the demo will be loaded later, when loading screen is drawn
}



// actually load the demo
void CDemo::loadDemo()
{
	clock_t t1 = clock();

	// scenes
	gSceneShared = new CSceneSharedStuff();
	gSceneMain = new CSceneMain( gSceneShared );
	gSceneInt = new CSceneInteractive( gSceneShared );
	gSceneScroller = new CSceneScroller();

	// post processes
	gPPReflBlur = new CPostProcess( RT_QUAD_TMP1, RT_QUAD_TMP2 );
	gPPDofBlur = new CPostProcess( RT_HALF_TMP1, RT_HALF_TMP2 );

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

	// DOF composite
	gQuadDOF = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	gQuadDOF->getParams().setEffect( *RGET_FX("compositeDOF") );
	gQuadDOF->getParams().addTexture( "tBase", *RGET_S_TEX(RT_FULLSCREEN) );
	gQuadDOF->getParams().addTexture( "tBlur1", *RGET_S_TEX(RT_DOF_1) );
	gQuadDOF->getParams().addTexture( "tBlur2", *RGET_S_TEX(RT_DOF_2) );

	// music
	music::init( mHwnd );

	// listeners and callbacks
	G_INPUTCTX->addListener( *this );
	gSetupGUI();
	gUIDlg->setRenderCallback( gUIRenderCallback );

	// if we loaded too fast, annoy users more and show the loading screen!
	// ok, it's at most 3 seconds; let's say that's needed because the beamers
	// switch resolutions slowly
	const float MIN_LOAD_TIME = 3.0f;
	clock_t t2 = clock();
	float loaded = float(t2-t1)/CLOCKS_PER_SEC;
	float towait = MIN_LOAD_TIME - loaded;
	if( towait > 0 ) {
		Sleep( towait * 1000 );
	}
}


// --------------------------------------------------------------------------
// Perform code (main loop)


static void	gStartMain()
{
	music::play( "data/sound/ic2005sound.ogg", true );
	gSceneShared->clearPieces();
	gCurScene = SCENE_MAIN;
}

static void	gStartScroller()
{
	music::play( "data/sound/ic2005loop.ogg", true );
	gSceneScroller->start( gDemoTimer.getTime() );
	gCurScene = SCENE_SCROLLER;
}

static void	gStartInteractiveMode()
{
	if( gCurScene != SCENE_SCROLLER )
		music::play( "data/sound/ic2005loop.ogg", true );
	gSceneInt->start( gDemoTimer.getTime(), *gUIDlg );
	gCurScene = SCENE_INTERACTIVE;
}


bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if( !gUIDlg )
		return false;

	bool done = false;

	if( tweaker::isVisible() ) {
		return tweaker::getDlg().msgProc( hwnd, msg, wparam, lparam );
	}

	done = gUIDlg->msgProc( hwnd, msg, wparam, lparam );
	if( done )
		return true;
	
	// track mouse...
	if( msg == WM_LBUTTONDOWN ) {
		// ...
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


static void gAdjustTime( float dt )
{
	gDemoTimer.update( time_value::fromsec( dt ) );
	music::setTime( gDemoTimer.getTimeS() );
}

void CDemo::onInputEvent( const CInputEvent& event )
{
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
		/*
		case DIK_0:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gPaused = !gPaused;
			break;

		case DIK_RETURN:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				++gCurScene;
				gCurScene %= SCENECOUNT;
				if( gCurScene == SCENE_MAIN )
					gStartMain();
				else if( gCurScene == SCENE_SCROLLER )
					gStartScroller();
				else if( gCurScene == SCENE_INTERACTIVE )
					gStartInteractiveMode();
			}
			break;
		*/
		case DIK_SPACE:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				if( gCurScene == SCENE_INTERACTIVE ) {
					gInputAttack = true;
				} else {
					gStartInteractiveMode();
				}
			}
			break;

		case DIK_F1:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED ) {
				// show tweaker UI
				if( !tweaker::isVisible() ) {
					tweaker::show();
				} else {
					tweaker::hide();
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

		/*
		case DIK_1:
			gAdjustTime( -dt*10 );
			break;
		case DIK_2:
			gAdjustTime( -dt*2 );
			break;
		case DIK_3:
			gAdjustTime( -dt );
			break;
		case DIK_4:
			gAdjustTime( -dt*0.8f );
			break;
		case DIK_5:
			gAdjustTime( dt*4 );
			break;
		case DIK_6:
			gAdjustTime( dt*20 );
			break;
		case DIK_7:
			gAdjustTime( dt*100 );
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
		gSceneInt->processInput( gInputTargetMoveSpeed, gInputTargetRotpeed, gInputAttack, animTime, gDemoTimer.getDeltaTimeS() );
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
	CD3DDevice& dx = CD3DDevice::getInstance();


	// very first perform - draw loading screen and return immediately
	static bool loadingScreenDone = false;
	if( !loadingScreenDone ) {
		loadingScreenDone = true;

		dx.clearTargets( true, true, false, 0xFFffffff, 1.0f, 0L );
		dx.sceneBegin();
		G_RENDERCTX->applyGlobalEffect();
		gUIDlg->onRender( 0.01f );
		dx.sceneEnd();

		gUIImgLoading->setVisible( false );
		return;
	}

	// second perform - actually load everything
	static bool loaded = false;
	if( !loaded ) {
		loaded = true;
		loadDemo();
	}


	G_INPUTCTX->perform();

	// timing
	double dt;
	static bool firstPerform = true;
	if( firstPerform || gPaused ) {
		if( firstPerform ) {
			// start music
			gStartMain();
		}
		dt = 0.0;
		firstPerform = false;
	} else {

		if( gCurScene == SCENE_MAIN ) {
			// during main demo, 100% synchronize to music
			float t = music::getTime();
			//float t = CSystemTimer::getInstance().getTimeS(); // TEST
			gDemoTimer.setTime( time_value::fromsec( t ) );
			dt = gDemoTimer.getDeltaTimeS();
			//CConsole::getChannel("system") << "music time: " << t << " demo time: " << gDemoTimer.getTimeS() << endl;
		} else {
			// during other parts, take time from system timer
			dt = CSystemTimer::getInstance().getDeltaTimeS();
			gDemoTimer.update( CSystemTimer::getInstance().getDeltaTime() );
		}
	}
	time_value demoTime = gDemoTimer.getTime();
	gTimeParam = demoTime.tosec();

	// figure out current scene
	D3DCOLOR clearColor = 0xFFffffff;
	bool	clearFlag = true;
	CScene* curScene = NULL;
	switch( gCurScene ) {
	case SCENE_MAIN:
		curScene = gSceneMain;
		clearColor = 0xFF404040;
		break;
	case SCENE_SCROLLER:
		curScene = gSceneScroller;
		clearColor = 0x00e0e0e0;
		break;
	case SCENE_INTERACTIVE:
		curScene = gSceneInt;
		clearColor = 0xFF404040;
		break;
	}
	assert( curScene );

	// update scene
	curScene->update( demoTime, dt );


	gWallVertCount = gWallTriCount = 0;


	gScreenFixUVs.set( 0.5f/dx.getBackBufferWidth(), 0.5f/dx.getBackBufferHeight(), 0.0f, 0.0f );
	
	
	// rendering options
	const tweaker::SOptions& options = tweaker::getOptions();

	// FPS
	if( options.showFPS ) {
		sprintf( buf, "FPS: %.1f", dx.getStats().getFPS() );
		gUILabFPS->setText( buf );
	}
	gUILabFPS->setVisible( options.showFPS );


	// use solid fill to render shadows/reflections - faster!
	dx.getStateManager().SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	curScene->getCamera().setOntoRenderContext();
	gCameraViewProjMatrix = G_RENDERCTX->getCamera().getViewProjMatrix();
	
	if( options.shadows ) {
		// render shadow map
		gShadowRender( *curScene );
		gShadowRender2( *curScene );
	}
	if( options.reflections ) {
		// render wall reflections
		gRenderWallReflections( *curScene );
	}

	// set fill mode from options
	dx.getStateManager().SetRenderState( D3DRS_FILLMODE, options.wireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	curScene->getCamera().setOntoRenderContext();
	gfx::textureProjectionWorld( gCameraViewProjMatrix, 1000.0f, 1000.0f, gViewTexProjMatrix );

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( clearFlag, true, false, clearColor, 1.0f, 0L );

	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();
	curScene->render( RM_NORMAL );
	G_RENDERCTX->perform();
	dx.sceneEnd();

	// DOF
	if( options.dof )
		gRenderDOF();

	// render GUI
	dx.sceneBegin();
	gUIDlg->onRender( CSystemTimer::getInstance().getDeltaTimeS() );
	if( tweaker::isVisible() ) {
		tweaker::getDlg().onRender( dt );
	}
	dx.sceneEnd();


	// manage scene transitions
	if( gCurScene == SCENE_MAIN && gSceneMain->isEnded() ) { 
		gStartScroller();
	}
	else if( gCurScene == SCENE_SCROLLER && gSceneScroller->isEnded() ) {
		gStartInteractiveMode();
	}

	music::update();
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	music::close();

	delete gDebugRenderer;

	tweaker::shutdown();
	safeDelete( gUIDlg );
	safeDelete( gPPReflBlur );
	safeDelete( gPPDofBlur );
	
	delete gSceneMain;
	delete gSceneScroller;
	delete gSceneInt;
	delete gSceneShared;
}
