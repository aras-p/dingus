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

float	gCamYaw;
float	gCamPitch;
float	gCamDist;

const int ENV_SH_ORDER = 3;
float	gEnvSHR[ENV_SH_ORDER*ENV_SH_ORDER];
float	gEnvSHG[ENV_SH_ORDER*ENV_SH_ORDER];
float	gEnvSHB[ENV_SH_ORDER*ENV_SH_ORDER];


// --------------------------------------------------------------------------
// objects


class CSceneEntity : public CMeshEntity {
public:
	CSceneEntity( const std::string& meshName, const std::string& fxName )
		: CMeshEntity( meshName )
	{
		mRenderMesh = new CRenderableMesh( getMesh(), CRenderableMesh::ALL_GROUPS, &mWorldMat.getOrigin(), 0 );
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

	CEffectParams& getFxParams() { return mRenderMesh->getParams(); }

private:
	CRenderableMesh*	mRenderMesh;
};

CSceneEntity*	gMesh;
SVector3	gSceneCenter;
float		gSceneRadius;


// --------------------------------------------------------------------------


void CALLBACK gUICallback( UINT evt, int ctrlID, CUIControl* ctrl )
{
}


void gSetSHEnvCoeffs( CEffectParams& ep )
{
	float* fLight[3] = { gEnvSHR, gEnvSHG, gEnvSHB };

	// Lighting environment coefficients
	D3DXVECTOR4 vCoefficients[3];

	// These constants are described in the article by Peter-Pike Sloan titled 
	// "Efficient Evaluation of Irradiance Environment Maps" in the book 
	// "ShaderX 2 - Shader Programming Tips and Tricks" by Wolfgang F. Engel.
	static const float s_fSqrtPI = ((float)sqrtf(D3DX_PI));
	const float fC0 = 1.0f/(2.0f*s_fSqrtPI);
	const float fC1 = (float)sqrt(3.0f)/(3.0f*s_fSqrtPI);
	const float fC2 = (float)sqrt(15.0f)/(8.0f*s_fSqrtPI);
	const float fC3 = (float)sqrt(5.0f)/(16.0f*s_fSqrtPI);
	const float fC4 = 0.5f*fC2;

	int iChannel;
	for( iChannel=0; iChannel<3; iChannel++ )
	{
		vCoefficients[iChannel].x = -fC1*fLight[iChannel][3];
		vCoefficients[iChannel].y = -fC1*fLight[iChannel][1];
		vCoefficients[iChannel].z =  fC1*fLight[iChannel][2];
		vCoefficients[iChannel].w =  fC0*fLight[iChannel][0] - fC3*fLight[iChannel][6];
	}

	ep.addVector4( "cAr", vCoefficients[0] );
	ep.addVector4( "cAg", vCoefficients[1] );
	ep.addVector4( "cAb", vCoefficients[2] );

	for( iChannel=0; iChannel<3; iChannel++ )
	{
		vCoefficients[iChannel].x =      fC2*fLight[iChannel][4];
		vCoefficients[iChannel].y =     -fC2*fLight[iChannel][5];
		vCoefficients[iChannel].z = 3.0f*fC3*fLight[iChannel][6];
		vCoefficients[iChannel].w =     -fC2*fLight[iChannel][7];
	}

	ep.addVector4( "cBr", vCoefficients[0] );
	ep.addVector4( "cBg", vCoefficients[1] );
	ep.addVector4( "cBb", vCoefficients[2] );

	vCoefficients[0].x = fC4*fLight[0][8];
	vCoefficients[0].y = fC4*fLight[1][8];
	vCoefficients[0].z = fC4*fLight[2][8];
	vCoefficients[0].w = 1.0f;

	ep.addVector4( "cC", vCoefficients[0] );
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
	
	gGlobalCullMode = D3DCULL_NONE;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );

	// --------------------------------
	// HDR environment

	CD3DCubeTexture* env = RGET_CUBETEX("HdrEnv_rnl");
	D3DXSHProjectCubeMap( ENV_SH_ORDER, env->getObject(), gEnvSHR, gEnvSHG, gEnvSHB );

	//SVector3 lightDir( -0.4f, 1.0f, -0.8f );
	//lightDir.normalize();
	//D3DXSHEvalDirectionalLight( ENV_SH_ORDER, &lightDir, 1.0f, 1.0f, 1.0f, gEnvSHR, gEnvSHG, gEnvSHB );

	// --------------------------------
	// scene

	gMesh = new CSceneEntity( "StAnna", "diffuseSHEnv" );
	gSetSHEnvCoeffs( gMesh->getFxParams() );

	gSceneCenter = gMesh->getAABB().getCenter();
	gSceneRadius = SVector3(gMesh->getAABB().getMax() - gMesh->getAABB().getMin()).length() * 0.5f;

	gCamYaw = 0.0f;
	gCamPitch = 0.1f;
	gCamDist = gSceneRadius * 2.0f;

	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );

	const int hctl = 16;
	const int hrol = 14;

	// fps
	{
		gUIDlgHUD->addStatic( 0, "", 5,  460, 200, 20, false, &gUIFPS );
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

		case DIK_LEFT:
			gCamYaw -= dt;
			break;
		case DIK_RIGHT:
			gCamYaw += dt;
			break;
		case DIK_UP:
			gCamPitch -= dt;
			break;
		case DIK_DOWN:
			gCamPitch += dt;
			break;
		case DIK_A:
			gCamDist -= dt*0.7f;
			break;
		case DIK_Z:
			gCamDist += dt*0.7f;
			break;
		}
	}

	gCamPitch = clamp( gCamPitch, -D3DX_PI*0.4f, D3DX_PI*0.4f );
	gCamDist = clamp( gCamDist, gSceneRadius * 0.5f, gSceneRadius * 4.0f );
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
	sprintf( buf, "fps: %6.1f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	SMatrix4x4& mc = gCamera.mWorldMat;
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() = gSceneCenter;
	mc.getOrigin() -= mc.getAxisZ() * gCamDist;

	const float camnear = gCamDist * 0.1f;
	const float camfar = gCamDist * 3.0f;
	const float camfov = D3DX_PI/4;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

	// render
	gCamera.setOntoRenderContext();

	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0xFF000000, 1.0f, 0L );
	dx.sceneBegin();
	G_RENDERCTX->applyGlobalEffect();

	gMesh->render();
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

	safeDelete( gMesh );
}
