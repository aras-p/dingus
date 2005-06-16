#include "stdafx.h"

#include "Demo.h"
#include "DemoResources.h"
#include "MeshEntity.h"
#include <dingus/renderer/RenderableMesh.h>
#include <dingus/gfx/GfxUtils.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>

#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/Line3.h>
#include <dingus/math/MathUtils.h>
#include <dingus/utils/Random.h>



// --------------------------------------------------------------------------
// Demo variables, constants, etc.

IDingusAppContext*	gAppContext;
int			gGlobalCullMode;	// global cull mode
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

	if( caps.getPShaderVersion() < CD3DDeviceCaps::PS_2_0 ) {
		errors.addError( "pixel shaders 2.0 required" );
		ok = false;
	}

	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 ) {
		if( vproc != CD3DDeviceCaps::VP_SW )
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
// Demo initialization

CCameraEntity	gCamera;
CCameraEntity	gLightCam;
SVector3		gTargetPos;
SVector3		gTargetVel;

CUIDialog*		gUIDlgHUD;
CUISlider*		gUISliderYaw;
CUISlider*		gUISliderPitch;
CUISlider*		gUISliderZoom;

CUICheckBox*	gUIChkDilate;

CUIStatic*		gUIFPS;

SMatrix4x4		gShadowProjMat;
SMatrix4x4		gLightViewProjMat; // View*Proj matrix for shadow light
SMatrix4x4		gViewTexProjMat;	// Texture projection matrix for main camera


// --------------------------------------------------------------------------
// objects

static const char* ENTITY_FX[RMCOUNT] = {
	"entity",
	"casterSoft",
	"casterSoftM",
	"caster",
};

class CSceneEntity : public CMeshEntity {
public:
	CSceneEntity( const std::string& meshName )
		: CMeshEntity( meshName )
	{
		for( int i = 0; i < RMCOUNT; ++i ) {
			mRenderMeshes[i] = new CRenderableMesh( getMesh(), 0, &mWorldMat.getOrigin(), 0 );
			CEffectParams& ep = mRenderMeshes[i]->getParams();
			ep.setEffect( *RGET_FX(ENTITY_FX[i]) );
			addMatricesToParams( ep );
		}
	}
	virtual ~CSceneEntity()
	{
		for( int i = 0; i < RMCOUNT; ++i )
			safeDelete( mRenderMeshes[i] );
	}

	void render( eRenderMode rm )
	{
		updateWVPMatrices();
		if( frustumCull() )
			return;
		G_RENDERCTX->attach( *mRenderMeshes[rm] );
	}

private:
	CRenderableMesh*	mRenderMeshes[RMCOUNT];
};

std::vector<CSceneEntity*>	gEntities;


static const char* ENTITY_NAMES[] = {
	"Cylinder",
	"Table",
	"Torus",
};

static const int ENTITY_NAME_COUNT = sizeof(ENTITY_NAMES) / sizeof(ENTITY_NAMES[0]);


void	gAddEntity( int index )
{
	CSceneEntity* e = new CSceneEntity( ENTITY_NAMES[index] );
	
	const CAABox& aabb = e->getAABB();
	SMatrix4x4& m = e->mWorldMat;

	m.getOrigin().set(
		gRandom.getFloat( -3.0f, 3.0f ),
		/*gRandom.getFloat( 0.0f, 2.0f ) */- aabb.getMin().y,
		gRandom.getFloat( -3.0f, 3.0f )
	);

	gEntities.push_back( e );
}

void	gRenderEntities( eRenderMode rm )
{
	for( int i = 0; i < gEntities.size(); ++i )
		gEntities[i]->render( rm );
}


// --------------------------------------------------------------------------

CRenderableMesh*	gMeshBox;



// soft shadows
CRenderableMesh*	gQuadGaussX;
CRenderableMesh*	gQuadGaussY;
CRenderableMesh*	gQuadBlur;

// mouse
float		gMouseX; // from -1 to 1
float		gMouseY; // from -1 to 1
SLine3		gMouseRay;


// interaction
int		gMeshPointed = -1;
int		gMeshDragged = -1;
float	gMeshDragStartDist;


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}


void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// common params
	
	stb.registerTexture( RT_SHADOW, *new CFixedTextureCreator(
			SZ_SHADOW, SZ_SHADOW, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT ) );
	stb.registerTexture( RT_SHADOWBLUR, *new CFixedTextureCreator(
			SZ_SHADOW, SZ_SHADOW, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT ) );

	ssb.registerSurface( RT_SHADOW, *new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOW),0) );
	ssb.registerSurface( RT_SHADOWBLUR, *new CTextureLevelSurfaceCreator(*RGET_S_TEX(RT_SHADOWBLUR),0) );
	
	ssb.registerSurface( RT_SHADOWZ, *new CFixedSurfaceCreator(
			SZ_SHADOW, SZ_SHADOW, true, D3DFMT_D16 ) );

	gGlobalCullMode = D3DCULL_CW;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mShadowProj", gShadowProjMat );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mViewTexProj", gViewTexProjMat );

	// --------------------------------
	// light
	
	gLightCam.mWorldMat.getAxisX().set( 1, 0, 0 );
	gLightCam.mWorldMat.getAxisY().set( 0, 0, 1 );
	gLightCam.mWorldMat.getAxisZ().set( 0,-1, 0 );
	gLightCam.mWorldMat.getOrigin().set( 0, 9.0f, 0 );
	gLightCam.setProjectionParams( D3DX_PI*0.5f, 1.0f, 0.1f, 20.0f );

	// --------------------------------
	// meshes

	gMeshBox = new CRenderableMesh( *RGET_MESH("Box"), 0 );
	gMeshBox->getParams().setEffect( *RGET_FX("box") );
	gMeshBox->getParams().addTexture( "tBase", *RGET_TEX("HellEdges") );
	gMeshBox->getParams().addTexture( "tShadow", *RGET_S_TEX(RT_SHADOWBLUR) );

	for( int i = 0; i < ENTITY_NAME_COUNT; ++i )
		gAddEntity( i );

	gTargetPos.set( 0, 0, 0 );
	gTargetVel.set(0,0,0);

	// gauss X shadowmap -> shadowblur
	gQuadGaussX = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	gQuadGaussX->getParams().setEffect( *RGET_FX("filterGaussX") );
	gQuadGaussX->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOW) );
	// gauss Y shadowblur -> shadowmap
	gQuadGaussY = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	gQuadGaussY->getParams().setEffect( *RGET_FX("filterGaussY") );
	gQuadGaussY->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOWBLUR) );
	// blur shadowmap -> shadowblur
	gQuadBlur = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	gQuadBlur->getParams().setEffect( *RGET_FX("filterPoisson") );
	gQuadBlur->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SHADOW) );


	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );

	const int hctl = 16;
	const int hrol = 14;

	// fps
	{
		gUIDlgHUD->addStatic( 0, "(wait)", 5,  460, 200, 20, false, &gUIFPS );
	}
	// zoom
	{
		gUIDlgHUD->addStatic( 0, "yaw ", 210,  5, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250,  5, 100, hctl, -180, 180, 10, false, &gUISliderYaw );
		gUIDlgHUD->addStatic( 0, "ptch", 210, 25, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250, 25, 100, hctl, 0, 90, 5, false, &gUISliderPitch );
		gUIDlgHUD->addStatic( 0, "zoom", 210, 45, 40, hctl );
		gUIDlgHUD->addSlider( 0, 250, 45, 100, hctl, 5, 20, 6, false, &gUISliderZoom );
	}
	//
	{
		gUIDlgHUD->addCheckBox( 0, "Preblur shadows", 5, 50, 100, 20, true, 0, false, &gUIChkDilate );
	}
}



// --------------------------------------------------------------------------
// Perform code (main loop)

bool CDemo::msgProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	bool done = false;
	if( gUIDlgHUD ) {
		done = gUIDlgHUD->msgProc( hwnd, msg, wparam, lparam );
		if( done )
			return true;
	}
	// track mouse...
	if( msg == WM_LBUTTONDOWN ) {
		gMeshDragged = gMeshPointed;
		if( gMeshDragged != -1 ) {
			SVector3 toNode = gEntities[gMeshDragged]->mWorldMat.getOrigin() - gCamera.mWorldMat.getOrigin();
			gMeshDragStartDist = gCamera.mWorldMat.getAxisZ().dot( toNode );
		}
	}
	if( msg == WM_MOUSEMOVE ) {
		CD3DDevice& dx = CD3DDevice::getInstance();
		gMouseX = (float(LOWORD(lparam)) / dx.getBackBufferWidth()) * 2 - 1;
		gMouseY = (float(HIWORD(lparam)) / dx.getBackBufferHeight()) * 2 - 1;
		if( !(wparam & MK_LBUTTON) ) {
			gMeshDragged = -1;
		}
	}
	return false;
}

void CDemo::onInputEvent( const CInputEvent& event )
{
	float dt = CSystemTimer::getInstance().getDeltaTimeS();

	if( event.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)event;
		switch( ke.getKeyCode() ) {
		case DIK_9:
			if( ke.getMode() == CKeyEvent::KEY_PRESSED )
				gShowStats = !gShowStats;
			break;

		case DIK_LEFT:	gTargetPos.x -= dt; break;
		case DIK_RIGHT:	gTargetPos.x += dt; break;
		case DIK_UP:	gTargetPos.z -= dt; break;
		case DIK_DOWN:	gTargetPos.z += dt; break;
		}
	}
}

void CDemo::onInputStage()
{
}


static int gMousePick()
{
	int index = -1;
	float minDist = DINGUS_BIG_FLOAT;

	// go through all meshes
	int i;
	int n = gEntities.size();
	for( i = 0; i < n; ++i ) {
		const SVector3& p = gEntities[i]->mWorldMat.getOrigin();
		const CAABox& aabb = gEntities[i]->getAABB();
		float radius = SVector3(aabb.getMax() - aabb.getMin()).length() * 0.4f;

		if( gMouseRay.project( p ) < 0.0f )
			continue;
		if( gMouseRay.distance( p ) > radius )
			continue;
		float dist2 = SVector3(p-gMouseRay.pos).lengthSq();
		if( dist2 < minDist ) {
			index = i;
			minDist = dist2;
		}
	}
	return index;
}


static void gInteraction() {
	if( gMeshDragged != -1 ) {
		SVector3 pt = gMouseRay.pos + gMouseRay.vec * gMeshDragStartDist;
		//SVector3 force = gMeshDragPoint - node[gMeshDragged].p;
		//node[gNodeDragged].f += force * 1.0f;

		const CAABox& aabb = gEntities[gMeshDragged]->getAABB();

		pt.x = clamp( pt.x, -5.0f - aabb.getMin().x, 5.0f - aabb.getMax().x );
		pt.y = clamp( pt.y,  0.0f - aabb.getMin().y, 10.0f - aabb.getMax().y );
		pt.z = clamp( pt.z, -5.0f - aabb.getMin().z, 5.0f - aabb.getMax().z );
	
		gEntities[gMeshDragged]->mWorldMat.getOrigin() = pt;
	}
}


static void gComputeTexProjs()
{
	gfx::textureProjectionView( G_RENDERCTX->getCamera().getCameraMatrix(), G_RENDERCTX->getCamera().getViewProjMatrix(), SZ_SHADOW, SZ_SHADOW, gViewTexProjMat );
	gfx::textureProjectionView( G_RENDERCTX->getCamera().getCameraMatrix(), gLightViewProjMat, SZ_SHADOW, SZ_SHADOW, gShadowProjMat );
}



void gRenderShadowMaps()
{
	//
	// compute tight fitting shadow projection

	/*
	const CAABox& aabb = gFEMMesh->getAABB();
	SVector3 target = aabb.getCenter();
	SVector3 size = aabb.getMax() - aabb.getMin();
	float radius = size.length() * 0.5f;
	float dist = SVector3(gLightCam.mWorldMat.getOrigin() - target).length();

	float fov = atan2f( radius, dist ) * 2.0f;

	gLightCam.mWorldMat.getAxisZ() = target - gLightCam.mWorldMat.getOrigin();
	gLightCam.mWorldMat.getAxisZ().normalize();
	gLightCam.mWorldMat.spaceFromAxisZ();

	gLightCam.setProjectionParams( fov, 1.0f, 0.1f, 20.0f );
	*/

	//
	// render shadow map

	CD3DDevice& dx = CD3DDevice::getInstance();

	gLightCam.setOntoRenderContext();
	gLightViewProjMat = G_RENDERCTX->getCamera().getViewProjMatrix();

	dx.setRenderTarget( RGET_S_SURF(RT_SHADOW) );
	dx.setZStencil( RGET_S_SURF(RT_SHADOWZ) );
	dx.clearTargets( true, true, false, 0xFF00ffff, 0.0f );

	// leave one texel padding
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 1; vp.Height = vp.Width = SZ_SHADOW-2;
	vp.MinZ = 0.0f; vp.MaxZ = 1.0f;
	dx.getDevice().SetViewport( &vp );
	
	dx.sceneBegin();
	gRenderEntities( RM_CASTERSOFT );
	G_RENDERCTX->applyGlobalEffect();
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );
	G_RENDERCTX->perform();
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	dx.sceneEnd();

	//
	// process shadow map
	dx.setZStencil( NULL );

	if( gUIChkDilate->isChecked() ) {
		// Gauss X shadowmap -> shadowblur
		dx.setRenderTarget( RGET_S_SURF(RT_SHADOWBLUR) );
		dx.sceneBegin();
		G_RENDERCTX->attach( *gQuadGaussX );
		G_RENDERCTX->perform();
		dx.sceneEnd();

		// Gauss Y shadowblur -> shadowmap
		dx.setRenderTarget( RGET_S_SURF(RT_SHADOW) );
		dx.sceneBegin();
		G_RENDERCTX->attach( *gQuadGaussY );
		G_RENDERCTX->perform();
		dx.sceneEnd();
	}
	
	// Poisson shadowmap -> shadowblur
	dx.setRenderTarget( RGET_S_SURF(RT_SHADOWBLUR) );
	dx.clearTargets( true, false, false, 0xFFffffff );
	dx.getDevice().SetViewport( &vp );
	dx.sceneBegin();
	G_RENDERCTX->attach( *gQuadBlur );
	G_RENDERCTX->perform();
	dx.sceneEnd();
}


/// Main loop code.
void CDemo::perform()
{
	CDynamicVBManager::getInstance().discard();

	G_INPUTCTX->perform();
	
	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	gTimeParam = float(t);

	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	char buf[100];
	sprintf( buf, "fps: %6.2f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	float zoom = gUISliderZoom->getValue();
	float yaw = D3DXToRadian( gUISliderYaw->getValue() );
	float pitch = D3DXToRadian( gUISliderPitch->getValue() );
	SMatrix4x4& mc = gCamera.mWorldMat;
	D3DXMatrixRotationYawPitchRoll( &mc, yaw, pitch, 0.0f );
	//gTargetPos = smoothCD( gTargetPos, gFEMMesh->getAABB().getCenter(), gTargetVel, 3.0f, dt );
	mc.getOrigin() = gTargetPos;
	mc.getOrigin() -= gCamera.mWorldMat.getAxisZ() * zoom;

	const float camnear = zoom * 0.2f;
	const float camfar = zoom * 5.0f;
	const float camfov = D3DX_PI/4;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

	// mouse ray
	SVector3 mouseRay = gCamera.getWorldRay( gMouseX, gMouseY );
	const SVector3& eyePos = gCamera.mWorldMat.getOrigin();
	gMouseRay.pos = eyePos;
	gMouseRay.vec = mouseRay;

	if( gMeshDragged == -1 ) {
		gMeshPointed = gMousePick();
	} else {
		gMeshPointed = gMeshDragged;
	}
	gInteraction();

	// render
	
	gRenderShadowMaps();
	
	gCamera.setOntoRenderContext();
	gComputeTexProjs();

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFF000000, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();

	gRenderEntities( RM_NORMAL );
	G_RENDERCTX->attach( *gMeshBox );
	
	G_RENDERCTX->perform();

	// render GUI
	gUIDlgHUD->onRender( dt );
	dx.sceneEnd();

	//dx.getDevice().StretchRect( RGET_S_SURF(RT_SHADOWBLUR)->getObject(), NULL, dx.getBackBuffer(), NULL, D3DTEXF_NONE );
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlgHUD );
	
	delete gMeshBox;
	stl_utils::wipe( gEntities );

	delete gQuadGaussX;
	delete gQuadGaussY;
	delete gQuadBlur;
}
