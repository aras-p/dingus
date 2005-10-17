#include "stdafx.h"

#include "Demo.h"
#include "MeshEntity.h"
#include <dingus/renderer/RenderableMesh.h>
#include <dingus/gfx/GfxUtils.h>

#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/math/Vector2.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/Vertices.h>


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
// Variables

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
// HDR stuff

// Samples we take in downsampling operations
const int MAX_SAMPLES = 16;

// BB sized 4xFP16 rendertarget
const char* RT_SCENE_HDR = "sceneHDR";

// First make BB size divisible by this; textures based on this size will
// be used later.
const int BB_DIVISIBLE_BY = 8;
int	gBackBufferCropWidth;
int	gBackBufferCropHeight;


// Scaled down scene RT, 4xFP16
const float SZ_SCENE_SCALED = 1.0f/4.0f;
const char* RT_SCENE_SCALED = "sceneScaled";

// Luminance textures that the viewer is currently adapted to
const int SZ_LUMINANCE = 1;
const D3DFORMAT FMT_LUMINANCE = D3DFMT_R32F; // geforces don't support R16F
const char* RT_LUM_CURR = "lumCurr";
const char* RT_LUM_LAST = "lumLast";

// Intermediate textures for computing luminance
const int NUM_TONEMAP_RTS = 4;
const char* RT_TONEMAP[NUM_TONEMAP_RTS] = {
	"toneMap0",
	"toneMap1",
	"toneMap2",
	"toneMap3",
};


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

// --------------------------------------------------------------------------

void gLoadMeshAO()
{
	CMesh& m = gMesh->getMesh();
	FILE* fao = fopen( "data/mesh/StAnna.ao", "rb" );
	BYTE* aodata = new BYTE[m.getVertexCount()];
	fread( aodata, m.getVertexCount(), 1, fao );
	SVertexXyzDiffuse* vb = (SVertexXyzDiffuse*)m.lockVBWrite();
	assert( m.getVertexStride() == sizeof(SVertexXyzDiffuse) );
	for( int i = 0; i < m.getVertexCount(); ++i ) {
		vb[i].diffuse &= 0x00ffffff;
		vb[i].diffuse |= (aodata[i] << 24);
	}
	m.unlockVBWrite();
	delete[] aodata;
	fclose( fao );
}


void CDemo::createResource() { gLoadMeshAO(); }
void CDemo::activateResource()
{
	int bbw = CD3DDevice::getInstance().getBackBufferWidth();
	int bbh = CD3DDevice::getInstance().getBackBufferHeight();
	gBackBufferCropWidth = bbw - bbw % BB_DIVISIBLE_BY;
	gBackBufferCropHeight = bbh - bbh % BB_DIVISIBLE_BY;
}
void CDemo::passivateResource() { }
void CDemo::deleteResource() { }


void CDemo::initialize( IDingusAppContext& appContext )
{
	gAppContext = &appContext;
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// rendertargets

	stb.registerTexture( RT_SCENE_HDR, *new CScreenBasedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,D3DFMT_A16B16G16R16F,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SCENE_HDR );
	stb.registerTexture( RT_SCENE_SCALED, *new CScreenBasedDivTextureCreator(SZ_SCENE_SCALED,SZ_SCENE_SCALED,BB_DIVISIBLE_BY,1,D3DUSAGE_RENDERTARGET,D3DFMT_A16B16G16R16F,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SCENE_SCALED );
	stb.registerTexture( RT_LUM_CURR, *new CFixedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,FMT_LUMINANCE,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_LUM_CURR );
	stb.registerTexture( RT_LUM_LAST, *new CFixedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,FMT_LUMINANCE,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_LUM_LAST );
	for( int i = 0; i < NUM_TONEMAP_RTS; ++i ) {
		int size = 1 << (2*i); // 1,4,16,...
		stb.registerTexture( RT_TONEMAP[i], *new CFixedTextureCreator(size,size,1,D3DUSAGE_RENDERTARGET,FMT_LUMINANCE,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_TONEMAP[i] );
	}

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

	gMesh = new CSceneEntity( "StAnna", "object" );
	gSetSHEnvCoeffs( gMesh->getFxParams() );
	gMesh->getFxParams().addCubeTexture( "tEnv", *env );
	gLoadMeshAO();
	CDeviceResourceManager::getInstance().addListener( *this );
	activateResource();

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
			gCamDist -= dt * gSceneRadius;
			break;
		case DIK_Z:
			gCamDist += dt * gSceneRadius;
			break;
		}
	}

	gCamPitch = clamp( gCamPitch, -D3DX_PI*0.4f, D3DX_PI*0.4f );
	gCamDist = clamp( gCamDist, gSceneRadius * 0.5f, gSceneRadius * 4.0f );
}

void CDemo::onInputStage()
{
}


void gDownscaleHDR4x()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	SVector2 smpOffsets[MAX_SAMPLES];
	
	// render into scaled down HDR target
	dx.setRenderTarget( RGET_S_SURF(RT_SCENE_SCALED) );

	int bbWidth = dx.getBackBufferWidth();
	int bbHeight = dx.getBackBufferHeight();

	//const D3DSURFACE_DESC* pBackBufferDesc = DXUTGetBackBufferSurfaceDesc();

	// Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
	// are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
	// to keep the dimensions evenly divisible by 8; this allows for precise 
	// control over sampling inside pixel shaders.
	//g_pEffect->SetTechnique("DownScale4x4");

	// Place the rectangle in the center of the back buffer surface
	RECT rectSrc;
	rectSrc.left = (bbWidth - gBackBufferCropWidth) / 2;
	rectSrc.top = (bbHeight - gBackBufferCropHeight) / 2;
	rectSrc.right = rectSrc.left + gBackBufferCropWidth;
	rectSrc.bottom = rectSrc.top + gBackBufferCropHeight;

	// Get the texture coordinates for the render target
	/*
	CoordRect coords;
	GetTextureCoords( g_pTexScene, &rectSrc, g_pTexSceneScaled, NULL, &coords );

	// Get the sample offsets used within the pixel shader
	GetSampleOffsets_DownScale4x4( bbWidth, bbHeight, smpOffsets );
	g_pEffect->SetValue("g_avSampleOffsets", smpOffsets, sizeof(smpOffsets));
	
	g_pd3dDevice->SetRenderTarget( 0, pSurfScaledScene );
	g_pd3dDevice->SetTexture( 0, g_pTexScene );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
   
	UINT uiPassCount, uiPass;		
	hr = g_pEffect->Begin(&uiPassCount, 0);
	if( FAILED(hr) )
		goto LCleanReturn;

	for (uiPass = 0; uiPass < uiPassCount; uiPass++)
	{
		g_pEffect->BeginPass(uiPass);

		// Draw a fullscreen quad
		DrawFullScreenQuad( coords );

		g_pEffect->EndPass();
	}

	g_pEffect->End();

	*/
}



static void gRender()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	// render scene to HDR rendertarget
	dx.setRenderTarget( RGET_S_SURF(RT_SCENE_HDR) );
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0x00000000, 1.0f, 0L );
	G_RENDERCTX->applyGlobalEffect();
	gMesh->render();
	G_RENDERCTX->perform();

    // Create a scaled copy of the scene
	gDownscaleHDR4x();
	
	/*
	// Measure luminance
	MeasureLuminance();
	
	// Calculate the current luminance adaptation level
	CalculateAdaptation();
	*/
	
	// Final composition to the LDR back buffer: tone mapping & blue shift.
	dx.setDefaultRenderTarget();
	
	// FinalScenePass technique
	//	float "g_fMiddleGray", g_fKeyValue;
	//  SetTexture( 0, g_pTexScene ); mag=point min=point
	//  --SetTexture( 1, g_apTexBloom[0] ); mag=linear min=linear
	//  --SetTexture( 2, g_apTexStar[0] ); mag=linear min=linear
	//  SetTexture( 3, g_pTexAdaptedLuminanceCur ); mag=point min=point
	//  each pass: DrawFullScreenQuad( 0.0f, 0.0f, 1.0f, 1.0f );
		
	dx.getDevice().StretchRect( RGET_S_SURF(RT_SCENE_HDR)->getObject(), 0, dx.getBackBuffer(), 0, D3DTEXF_NONE );
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

	// camera
	SMatrix4x4& mc = gCamera.mWorldMat;
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() = gSceneCenter;
	mc.getOrigin() -= mc.getAxisZ() * gCamDist;
	const float camnear = gCamDist * 0.1f;
	const float camfar = gCamDist * 3.0f;
	const float camfov = D3DX_PI/4;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );
	gCamera.setOntoRenderContext();

	// render
	dx.sceneBegin();
	gRender();
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
