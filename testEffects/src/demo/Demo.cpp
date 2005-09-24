#include "stdafx.h"

#include "Demo.h"
#include "MeshEntity.h"
#include <dingus/renderer/RenderableMesh.h>
#include <dingus/gfx/GfxUtils.h>

#include <dingus/gfx/gui/Gui.h>
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

CUIDialog*		gUIDlgHUD;
CUIStatic*		gUIFPS;

// --------------------------------------------------------------------------
// objects


class CSceneEntity : public CMeshEntity {
public:
	CSceneEntity( const std::string& meshName, const std::string& fxName, int priority )
		: CMeshEntity( meshName )
	{
		mRenderMesh = new CRenderableMesh( getMesh(), 0, &mWorldMat.getOrigin(), priority );
		CEffectParams& ep = mRenderMesh->getParams();
		ep.setEffect( *RGET_FX(fxName) );
		addMatricesToParams( ep );
	}
	virtual ~CSceneEntity()
	{
		safeDelete( mRenderMesh );
	}

	void render()
	{
		updateWVPMatrices();
		G_RENDERCTX->attach( *mRenderMesh );
	}

private:
	CRenderableMesh*	mRenderMesh;
};

std::vector<CSceneEntity*>	gEntities;


static const char* ENTITY_NAMES[] = {
	"Cylinder",
	"Table",
	"Torus",
};
static const int ENTITY_NAME_COUNT = sizeof(ENTITY_NAMES) / sizeof(ENTITY_NAMES[0]);


static const char* FX_NAMES[] = {
	"color",
	"colorBlend",
	"colorAdd",
	"colorMul",
};
static int FX_PRIORITY[] = {
	0,
	1,
	2,
	3,
};
static const int FX_COUNT = sizeof(FX_NAMES) / sizeof(FX_NAMES[0]);


void	gAddEntity()
{
	int meshIndex = gRandom.getInt( 0, ENTITY_NAME_COUNT );
	int fxIndex = gRandom.getInt( 0, FX_COUNT );
	CSceneEntity* e = new CSceneEntity( ENTITY_NAMES[meshIndex], FX_NAMES[fxIndex], FX_PRIORITY[fxIndex] );
	
	D3DXMatrixRotationYawPitchRoll( &e->mWorldMat,
		gRandom.getFloat(0,D3DX_PI*2), gRandom.getFloat(-D3DX_PI,D3DX_PI), gRandom.getFloat(0,D3DX_PI)
	);
	e->mWorldMat.getOrigin().set(
		gRandom.getFloat( -20.0f, 20.0f ),
		gRandom.getFloat( -20.0f, 20.0f ),
		gRandom.getFloat( -10.0f, 30.0f )
	);

	gEntities.push_back( e );
}

void	gRenderEntities()
{
	int n = gEntities.size();
	for( int i = 0; i < n; ++i )
		gEntities[i]->render();
}


// --------------------------------------------------------------------------


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
	
	gGlobalCullMode = D3DCULL_CW;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );

	// --------------------------------
	// meshes

	const int ENTITY_COUNT = 1000;
	for( int i = 0; i < ENTITY_COUNT; ++i )
		gAddEntity();

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
		}
	}
}

void CDemo::onInputStage()
{
}



/// Main loop code.
void CDemo::perform()
{
	G_INPUTCTX->perform();
	
	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	gTimeParam = float(t);

	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	char buf[100];
	sprintf( buf, "fps: %6.2f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	SMatrix4x4& mc = gCamera.mWorldMat;
	mc.getOrigin().z = -10.0f;

	const float camnear = 0.1f;
	const float camfar = 50.0f;
	const float camfov = D3DX_PI/4;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

	// render
	gCamera.setOntoRenderContext();

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFF000000, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();

	gRenderEntities();
	G_RENDERCTX->perform();

	// render GUI
	gUIDlgHUD->onRender( dt );
	dx.sceneEnd();
}



// --------------------------------------------------------------------------
// Cleanup


void CDemo::shutdown()
{
	safeDelete( gUIDlgHUD );
	
	stl_utils::wipe( gEntities );
}
