#include "stdafx.h"

#include "Demo.h"
#include "Entity.h"
#include <dingus/renderer/RenderableMesh.h>
#include <dingus/renderer/RenderableQuad.h>
#include <dingus/gfx/GfxUtils.h>

#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>
#include <dingus/math/Vector2.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/Vertices.h>
#include <dingus/dxutils/d3dsettings.h>


// --------------------------------------------------------------------------
// Demo variables, constants, etc.

IDingusAppContext*	gAppContext;
int			gGlobalCullMode;	// global cull mode
float		gTimeParam;			// time parameter for effects
float		gDeltaTimeParam;	// delta time parameter for effects

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
	if( !caps.hasFloatTextures() ) {
		errors.addError( "floating point texture support required" );
		ok = false;
	}
	if( caps.getVShaderVersion() < CD3DDeviceCaps::VS_1_1 ) {
		if( vproc != CD3DDeviceCaps::VP_SW )
			ok = false;
	}
	return ok;
}

void CDemo::initD3DSettingsPref( SD3DSettingsPref& pref )
{
	// TBD: does not work now!
	//pref.fsaaType = D3DMULTISAMPLE_4_SAMPLES;
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
CUISlider*		gUISldMiddleGray;
CUIStatic*		gUILabMiddleGray;

CUISlider*		gUISldLightIntensity;
CUISlider*		gUISldLightYaw;
CUISlider*		gUISldLightPitch;


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

// BB sized RGBE8 rendertarget
const char* RT_SCENE = "scene";

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
const D3DFORMAT FMT_LUMINANCE = D3DFMT_R32F; // some geforces don't support R16F

const char* RT_LUM[2] = {
	"lum1",
	"lum2",
};
int	gCurrLuminanceIndex = 0; // into RT_LUM texture


// Intermediate textures for computing luminance
const int NUM_TONEMAP_RTS = 4;
const char* RT_TONEMAP[NUM_TONEMAP_RTS] = {
	"toneMap0",
	"toneMap1",
	"toneMap2",
	"toneMap3",
};


const D3DFORMAT FMT_BRIGHT_PASS = D3DFMT_A8R8G8B8;
const char* RT_BRIGHT_PASS = "brightpass";

// Textures used for bloom
const char* RT_BLOOM = "bloom";

const int NUM_BLOOM_TMP_TEXTURES = 2;
const char* RT_BLOOM_TMP[NUM_BLOOM_TMP_TEXTURES] = {
	"bloomTmp0",
	"bloomTmp1",
};


SVector2 gSampleOffsets[MAX_SAMPLES];
CRenderableQuad*	gQuadDownsampleBB;
CRenderableQuad*	gQuadSampleAvgLum;
CRenderableQuad*	gQuadResampleAvgLum;
CRenderableQuad*	gQuadResampleAvgLumExp;
CRenderableQuad*	gQuadCalcAdaptedLum;
CRenderableQuad*	gQuadBrightPass;
CRenderableQuad*	gQuadIterativeBloom;
CRenderableQuad*	gQuadFinalScenePass;

CRenderableMesh*	gMeshSkybox;


float	gHDRMiddleGray;


// --------------------------------------------------------------------------
// objects

CRenderableMesh*	gMeshNormal;
CRenderableMesh*	gMeshCaster;

CRenderableMesh*	gMeshGround;
CRenderableMesh*	gMeshLight;
SVector3			gLightPos;

SVector3	gSceneCenter;
float		gSceneRadius;

// --------------------------------------------------------------------------
// shadow mapping

bool			gUseDSTShadows;

CCameraEntity	gShadowLight;
SMatrix4x4		gShadowLightViewProj;
SMatrix4x4		gShadowTexProj;
float			gShadowYaw, gShadowPitch, gShadowDist;
float			gLightIntensity;
//float			gShadowBias;


const int SZ_SHADOWMAP = 2048;
const char* RT_SHADOW = "shadow";
const char* RT_SHADOWZ = "shadowZ";
const char* RT_SHADOW_DSTRT = "shadowDstRT";

void gShadowInit()
{
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	gUseDSTShadows = CD3DDevice::getInstance().getCaps().hasShadowMaps();

	if( gUseDSTShadows ) {
		CEffectBundle::getInstance().setMacro( "DST_SHADOWS", "1" );
		stb.registerTexture( RT_SHADOW_DSTRT, *new CFixedTextureCreator(SZ_SHADOWMAP,SZ_SHADOWMAP,1,D3DUSAGE_RENDERTARGET,D3DFMT_R5G6B5,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SHADOW_DSTRT );
		stb.registerTexture( RT_SHADOW, *new CFixedTextureCreator(SZ_SHADOWMAP,SZ_SHADOWMAP,1,D3DUSAGE_DEPTHSTENCIL,D3DFMT_D24X8,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SHADOW );
	} else {
		stb.registerTexture( RT_SHADOW, *new CFixedTextureCreator(SZ_SHADOWMAP,SZ_SHADOWMAP,1,D3DUSAGE_RENDERTARGET,D3DFMT_R32F,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SHADOW );
		ssb.registerSurface( RT_SHADOWZ, *new CFixedSurfaceCreator(SZ_SHADOWMAP, SZ_SHADOWMAP, true, D3DFMT_D24S8 ) );
	}

	//G_RENDERCTX->getGlobalParams().addFloatRef( "fShadowBias", &gShadowBias );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fLightIntensity", &gLightIntensity );
	G_RENDERCTX->getGlobalParams().addVector3Ref( "vLightDir", gShadowLight.mWorldMat.getAxisZ() );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mLightViewProj", gShadowLightViewProj );
	G_RENDERCTX->getGlobalParams().addMatrix4x4Ref( "mShadowProj", gShadowTexProj );
}


void gShadowRender()
{
	// position the light camera
	gLightIntensity = gUISldLightIntensity->getValue() * 0.1f;
	gShadowYaw = D3DXToRadian( gUISldLightYaw->getValue() );
	gShadowPitch = D3DXToRadian( gUISldLightPitch->getValue() );
	D3DXMatrixRotationYawPitchRoll( &gShadowLight.mWorldMat, gShadowYaw, gShadowPitch, 0.0f );
	gShadowLight.mWorldMat.getOrigin() = gSceneCenter;
	gShadowLight.mWorldMat.getOrigin() -= gShadowLight.mWorldMat.getAxisZ() * gShadowDist;
	gLightPos = gSceneCenter;
	gLightPos -= gShadowLight.mWorldMat.getAxisZ() * gShadowDist * 20.0f;

	gShadowDist = gSceneRadius * 1.5f;
	gShadowLight.setOrthoParams( gSceneRadius*2, gSceneRadius*2, gSceneRadius*0.3f, gSceneRadius*4.0f );
	// update matrices
	gShadowLight.setOntoRenderContext();
	gShadowLightViewProj = G_RENDERCTX->getCamera().getViewProjMatrix();
	gfx::textureProjectionWorld( gShadowLightViewProj, SZ_SHADOWMAP, SZ_SHADOWMAP, gShadowTexProj );

	//gShadowBias = gSceneRadius * 0.001f;

	// render shadow map
	CD3DDevice& dx = CD3DDevice::getInstance();
	if( gUseDSTShadows ) {
		dx.setRenderTarget( RGET_S_SURF(RT_SHADOW_DSTRT) );
		dx.setZStencil( RGET_S_SURF(RT_SHADOW) );
		dx.getStateManager().SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		dx.clearTargets( false, true, false, 0xFFffffff, 1.0f );
	} else {
		dx.setRenderTarget( RGET_S_SURF(RT_SHADOW) );
		dx.setZStencil( RGET_S_SURF(RT_SHADOWZ) );
		dx.clearTargets( true, true, false, 0xFFffffff, 1.0f );
	}

	G_RENDERCTX->applyGlobalEffect();
	G_RENDERCTX->attach( *gMeshCaster );
	G_RENDERCTX->perform();

	if( gUseDSTShadows ) {
		dx.getStateManager().SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA );
	}
}


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

void gClearSurface( CD3DSurface* surf )
{
	CD3DDevice::getInstance().getDevice().ColorFill( surf->getObject(), NULL, 0 );
}

void gLoadMeshAO()
{
	CMesh& m = gMeshNormal->getMesh();
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
	gClearSurface( RGET_S_SURF(RT_LUM[0]) );
	gClearSurface( RGET_S_SURF(RT_LUM[1]) );
}
void CDemo::passivateResource() { }
void CDemo::deleteResource() { }



void CDemo::initialize( IDingusAppContext& appContext )
{
	int i;
	gAppContext = &appContext;
	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	CSharedSurfaceBundle& ssb = CSharedSurfaceBundle::getInstance();

	CD3DDevice& dx = CD3DDevice::getInstance();

	G_INPUTCTX->addListener( *this );

	// --------------------------------
	// rendertargets

	stb.registerTexture( RT_SCENE, *new CScreenBasedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SCENE );
	stb.registerTexture( RT_SCENE_SCALED, *new CScreenBasedDivTextureCreator(SZ_SCENE_SCALED,SZ_SCENE_SCALED,BB_DIVISIBLE_BY,1,D3DUSAGE_RENDERTARGET,D3DFMT_A16B16G16R16F,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_SCENE_SCALED );
	stb.registerTexture( RT_LUM[0], *new CFixedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,FMT_LUMINANCE,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_LUM[0] );
	gClearSurface( RGET_S_SURF(RT_LUM[0]) );
	stb.registerTexture( RT_LUM[1], *new CFixedTextureCreator(1,1,1,D3DUSAGE_RENDERTARGET,FMT_LUMINANCE,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_LUM[1] );
	gClearSurface( RGET_S_SURF(RT_LUM[1]) );
	for( i = 0; i < NUM_TONEMAP_RTS; ++i ) {
		int size = 1 << (2*i); // 1,4,16,...
		stb.registerTexture( RT_TONEMAP[i], *new CFixedTextureCreator(size,size,1,D3DUSAGE_RENDERTARGET, FMT_LUMINANCE,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_TONEMAP[i] );
	}
	stb.registerTexture( RT_BRIGHT_PASS, *new CScreenBasedDivTextureCreator(SZ_SCENE_SCALED,SZ_SCENE_SCALED,BB_DIVISIBLE_BY,1,D3DUSAGE_RENDERTARGET,FMT_BRIGHT_PASS,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_BRIGHT_PASS );
	stb.registerTexture( RT_BLOOM, *new CScreenBasedDivTextureCreator(SZ_SCENE_SCALED,SZ_SCENE_SCALED,BB_DIVISIBLE_BY,1,D3DUSAGE_RENDERTARGET,FMT_BRIGHT_PASS,D3DPOOL_DEFAULT) );
	DINGUS_REGISTER_STEX_SURFACE( ssb, RT_BLOOM );
	for( i = 0; i < NUM_BLOOM_TMP_TEXTURES; ++i ) {
		stb.registerTexture( RT_BLOOM_TMP[i], *new CScreenBasedDivTextureCreator(SZ_SCENE_SCALED,SZ_SCENE_SCALED,BB_DIVISIBLE_BY,1,D3DUSAGE_RENDERTARGET,FMT_BRIGHT_PASS,D3DPOOL_DEFAULT) );
		DINGUS_REGISTER_STEX_SURFACE( ssb, RT_BLOOM_TMP[i] );
	}
	gShadowInit();

	// --------------------------------
	// common params
	
	gGlobalCullMode = D3DCULL_NONE;
	G_RENDERCTX->getGlobalParams().addIntRef( "iCull", &gGlobalCullMode );
	G_RENDERCTX->getGlobalParams().addFloatRef( "fTime", &gTimeParam );

	// --------------------------------
	// HDR environment

	//CD3DCubeTexture* env = RGET_CUBETEX("HdrEnv_DH0001");
	CD3DCubeTexture* env = RGET_CUBETEX("HdrEnv_StPeters");
	D3DXSHProjectCubeMap( ENV_SH_ORDER, env->getObject(), gEnvSHR, gEnvSHG, gEnvSHB );

	//SVector3 lightDir( -0.4f, 1.0f, -0.8f );
	//lightDir.normalize();
	//D3DXSHEvalDirectionalLight( ENV_SH_ORDER, &lightDir, 1.0f, 1.0f, 1.0f, gEnvSHR, gEnvSHG, gEnvSHB );

	// --------------------------------
	// scene

	gMeshLight = new CRenderableMesh( *RGET_MESH("Sphere"), 0, NULL, -1 );
	gMeshLight->getParams().setEffect( *RGET_FX("objectLight") );
	gMeshLight->getParams().addVector3Ref( "vPos", gLightPos );

	gMeshCaster = new CRenderableMesh( *RGET_MESH("StAnnaPos"), 0, NULL, 0 );
	gMeshCaster->getParams().setEffect( *RGET_FX("caster") );

	gMeshGround = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	gMeshGround->getParams().setEffect( *RGET_FX("objectGround") );
	gSetSHEnvCoeffs( gMeshGround->getParams() );
	gMeshGround->getParams().addTexture( "tShadow", *RGET_S_TEX(RT_SHADOW) );
	gMeshGround->getParams().addFloatRef( "fSize", &gSceneRadius );
	gMeshGround->getParams().addVector3Ref( "vPos", gSceneCenter );

	gMeshNormal = new CRenderableMesh( *RGET_MESH("StAnna"), 0, NULL, 0 );
	gMeshNormal->getParams().setEffect( *RGET_FX("object") );
	gSetSHEnvCoeffs( gMeshNormal->getParams() );
	gMeshNormal->getParams().addCubeTexture( "tEnv", *env );
	gMeshNormal->getParams().addTexture( "tShadow", *RGET_S_TEX(RT_SHADOW) );
	gLoadMeshAO();
	CDeviceResourceManager::getInstance().addListener( *this );
	activateResource();

	gSceneCenter = gMeshNormal->getMesh().getTotalAABB().getCenter();
	gSceneRadius = SVector3(gMeshNormal->getMesh().getTotalAABB().getMax() - gMeshNormal->getMesh().getTotalAABB().getMin()).length() * 0.5f;

	gCamYaw = 0.0f;
	gCamPitch = 0.1f;
	gCamDist = gSceneRadius * 2.0f;

	// --------------------------------
	// postprocessing quads

	gQuadDownsampleBB = new CRenderableQuad( renderquad::SCoordRect() );
	gQuadDownsampleBB->getParams().setEffect( *RGET_FX("downsample4x4") );
	gQuadDownsampleBB->getParams().addPtr( "vSmpOffsets", sizeof(gSampleOffsets[0])*4, gSampleOffsets );
	gQuadDownsampleBB->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SCENE) );

	gQuadSampleAvgLum = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadSampleAvgLum->getParams().setEffect( *RGET_FX("sampleAvgLum") );
	gQuadSampleAvgLum->getParams().addPtr( "vSmpOffsets", sizeof(gSampleOffsets[0])*9, gSampleOffsets );
	gQuadSampleAvgLum->getParams().addTexture( "tBase", *RGET_S_TEX(RT_SCENE_SCALED) );
	
	gQuadResampleAvgLum = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadResampleAvgLum->getParams().setEffect( *RGET_FX("resampleAvgLum") );
	gQuadResampleAvgLum->getParams().addPtr( "vSmpOffsets", sizeof(gSampleOffsets), gSampleOffsets );
	
	gQuadResampleAvgLumExp = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadResampleAvgLumExp->getParams().setEffect( *RGET_FX("resampleAvgLumExp") );
	gQuadResampleAvgLumExp->getParams().addPtr( "vSmpOffsets", sizeof(gSampleOffsets), gSampleOffsets );
	
	gQuadCalcAdaptedLum = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadCalcAdaptedLum->getParams().setEffect( *RGET_FX("calcAdaptedLum") );
	gQuadCalcAdaptedLum->getParams().addFloatRef( "fDeltaTime", &gDeltaTimeParam );
	
	gQuadBrightPass = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadBrightPass->getParams().setEffect( *RGET_FX("brightPass") );
	gQuadBrightPass->getParams().addFloatRef( "fMiddleGray", &gHDRMiddleGray );
	
	gQuadIterativeBloom = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadIterativeBloom->getParams().setEffect( *RGET_FX("bloom") );
	gQuadIterativeBloom->getParams().addPtr( "vSmpOffsets", sizeof(gSampleOffsets[0])*4, gSampleOffsets );
	
	gQuadFinalScenePass = new CRenderableQuad( renderquad::SCoordRect(0,0,1,1) );
	gQuadFinalScenePass->getParams().setEffect( *RGET_FX("finalScenePass") );
	gQuadFinalScenePass->getParams().addFloatRef( "fMiddleGray", &gHDRMiddleGray );

	CMesh* msky = RGET_MESH("skybox");
	gMeshSkybox = new CRenderableMesh( *msky, 0, NULL, -2 );
	gMeshSkybox->getParams().setEffect( *RGET_FX("skybox") );
	gMeshSkybox->getParams().addCubeTexture( "tEnv", *env );
	gMeshSkybox->getParams().addVector3Ref( "vPos", gCamera.mWorldMat.getOrigin() );


	// --------------------------------
	// GUI

	gUIDlgHUD = new CUIDialog();
	gUIDlgHUD->setCallback( gUICallback );

	const int hctl = 16;

	// FPS
	gUIDlgHUD->addStatic( 0, "", 5,  460, 200, 20, false, &gUIFPS );

	// tweakables
	gUIDlgHUD->addStatic( 0, "Middle gray:", 5,  5, 100, hctl );
	gUIDlgHUD->addSlider( 0, 100, 5, 70, hctl, 0, 4, 2, false, &gUISldMiddleGray );
	gUIDlgHUD->addStatic( 0, "", 180,  5, 100, hctl, false, &gUILabMiddleGray );
	
	gUIDlgHUD->addStatic( 0, "Light intens:", 300, 5, 100, hctl );
	gUIDlgHUD->addSlider( 0, 360, 5, 120, hctl, 1, 100, 10, false, &gUISldLightIntensity );
	gUIDlgHUD->addStatic( 0, "Light yaw:", 300, 25, 100, hctl );
	gUIDlgHUD->addSlider( 0, 360, 25, 120, hctl, 0, 360, 45, false, &gUISldLightYaw );
	gUIDlgHUD->addStatic( 0, "Light pitch:", 300, 45, 100, hctl );
	gUIDlgHUD->addSlider( 0, 360, 45, 120, hctl, 10, 80, 60, false, &gUISldLightPitch );
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
	gCamDist = clamp( gCamDist, gSceneRadius * 0.3f, gSceneRadius * 4.0f );
}

void CDemo::onInputStage()
{
}

// --------------------------------------------------------------------------


void gGetDownscale4x4SampleOffsets( DWORD dwWidth, DWORD dwHeight, D3DXVECTOR2 smpOffsets[] )
{
	assert( smpOffsets );
	
	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;
	
	// Sample from the 16 surrounding points. Since the center point will be
	// in the exact center of 16 texels, a 0.5f offset is needed to specify
	// a texel center.
	int index=0;
	for( int y=0; y < 4; ++y )
	{
		for( int x=0; x < 4; ++x )
		{
			smpOffsets[ index ].x = (x - 1.5f) * tU;
			smpOffsets[ index ].y = (y - 1.5f) * tV;
			index++;
		}
	}
}

void gGetDownscale4x4SampleOffsetsLinear( DWORD dwWidth, DWORD dwHeight, SVector2 smpOffsets[] )
{
	assert( smpOffsets );
	
	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;
	
	// Sample from the 16 surrounding points. Since the center point will be
	// in the exact center of 16 texels, a 0.5f offset is needed to specify
	// a texel center. Use bilinear blending so that we only have to
	// sample 4 locations.
	smpOffsets[0].set( -1.0f*tU, -1.0f*tV );
	smpOffsets[1].set( -1.0f*tU,  1.0f*tV );
	smpOffsets[2].set(  1.0f*tU, -1.0f*tV );
	smpOffsets[3].set(  1.0f*tU,  1.0f*tV );
}


// scale backbuffer down into small HDR target
void gDownscaleHDR4x()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	int bbWidth = dx.getBackBufferWidth();
	int bbHeight = dx.getBackBufferHeight();

	// Create a 1/4 x 1/4 scale copy of the HDR texture. Since bloom textures
	// are 1/8 x 1/8 scale, border texels of the HDR texture will be discarded 
	// to keep the dimensions evenly divisible by 8; this allows for precise 
	// control over sampling inside pixel shaders.

	// place the rectangle in the center of the back buffer surface
	RECT rectSrc;
	rectSrc.left = (bbWidth - gBackBufferCropWidth) / 2;
	rectSrc.top = (bbHeight - gBackBufferCropHeight) / 2;
	rectSrc.right = rectSrc.left + gBackBufferCropWidth;
	rectSrc.bottom = rectSrc.top + gBackBufferCropHeight;

	// get the texture coordinates for the render target
	renderquad::SCoordRect coords;
	renderquad::calcCoordRect( *RGET_S_TEX(RT_SCENE), &rectSrc, *RGET_S_TEX(RT_SCENE_SCALED), NULL, coords );
	gQuadDownsampleBB->setUVRect( coords );

	// get the sample offsets used within the pixel shader
	gGetDownscale4x4SampleOffsetsLinear( bbWidth, bbHeight, gSampleOffsets );
	dx.setRenderTarget( RGET_S_SURF(RT_SCENE_SCALED) );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadDownsampleBB );
	G_RENDERCTX->directEnd();
}


// measure the average log luminance in the scene.
void gMeasureLuminance()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	int x, y;
	
	D3DSURFACE_DESC desc;
	int curRT = NUM_TONEMAP_RTS-1;
	RGET_S_TEX(RT_TONEMAP[curRT])->getObject()->GetLevelDesc( 0, &desc );
	
	// initialize the sample offsets for the initial luminance pass.
	assert( MAX_SAMPLES >= 9 );
	float tU, tV;
	tU = 1.0f / (3.0f * desc.Width);
	tV = 1.0f / (3.0f * desc.Height);
	
	int index = 0;
	for( x = -1; x <= 1; ++x ) {
		for( y = -1; y <= 1; ++y ) {
			gSampleOffsets[index].set( x * tU, y * tV );
			++index;
		}
	}
	
	// After this pass, the RT_TONEMAP[NUM_TONEMAP_RTS-1] will contain
	// a scaled, grayscale copy of the HDR scene. Individual texels contain
	// the log of average luminance values for points sampled on the HDR
	// texture.
	dx.setRenderTarget( RGET_S_SURF(RT_TONEMAP[curRT]) );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadSampleAvgLum );
	G_RENDERCTX->directEnd();

	--curRT;
	
	// initialize the sample offsets for the iterative luminance passes
	while( curRT > 0 )
	{
		CD3DTexture* rtCurr1 = RGET_S_TEX(RT_TONEMAP[curRT+1]);
		rtCurr1->getObject()->GetLevelDesc( 0, &desc );
		gGetDownscale4x4SampleOffsets( desc.Width, desc.Height, gSampleOffsets );
		
		// Each of these passes continue to scale down the log of average
		// luminance texture created above, storing intermediate results in 
		// RT_TONEMAP[1] through RT_TONEMAP[NUM_TONEMAP_RTS-1].

		dx.setRenderTarget( RGET_S_SURF(RT_TONEMAP[curRT]) );
		dx.getStateManager().SetTexture( 0, rtCurr1->getObject() );
		dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		G_RENDERCTX->directBegin();
		G_RENDERCTX->directRender( *gQuadResampleAvgLum );
		G_RENDERCTX->directEnd();
		--curRT;
	}
	
	// downsample to 1x1
	// Perform the final pass of the average luminance calculation. This pass
	// scales the 4x4 log of average luminance texture from above and performs
	// an exp() operation to return a single texel corresponding to the average
	// luminance of the scene in RT_TONEMAP[0].
	CD3DTexture* rtCurr1 = RGET_S_TEX(RT_TONEMAP[1]);
	rtCurr1->getObject()->GetLevelDesc( 0, &desc );
	gGetDownscale4x4SampleOffsets( desc.Width, desc.Height, gSampleOffsets );
	
	dx.setRenderTarget( RGET_S_SURF(RT_TONEMAP[0]) );
	dx.getStateManager().SetTexture( 0, rtCurr1->getObject() );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadResampleAvgLumExp );
	G_RENDERCTX->directEnd();
}


void gCalculateAdaptation()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	// swap current & last luminance
	int prevLumIndex = gCurrLuminanceIndex;
	gCurrLuminanceIndex = (gCurrLuminanceIndex+1) % 2;
	
	// This simulates the light adaptation that occurs when moving from a 
	// dark area to a bright area, or vice versa. The
	// RT_LUM[gCurrLuminanceIndex] texture stores a single texel
	// corresponding to the user's adapted level.

	dx.setRenderTarget( RGET_S_SURF(RT_LUM[gCurrLuminanceIndex]) );
	dx.getStateManager().SetTexture( 0, RGET_S_TEX(RT_LUM[prevLumIndex])->getObject() );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetTexture( 1, RGET_S_TEX(RT_TONEMAP[0])->getObject() );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadCalcAdaptedLum );
	G_RENDERCTX->directEnd();
}


void gBrightPass()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	// The bright-pass filter removes everything from the scene except
	// lights and bright reflections
	dx.setRenderTarget( RGET_S_SURF(RT_BRIGHT_PASS) );
	dx.getStateManager().SetTexture( 0, RGET_S_TEX(RT_SCENE_SCALED)->getObject() );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetTexture( 1, RGET_S_TEX(RT_LUM[gCurrLuminanceIndex])->getObject() );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadBrightPass );
	G_RENDERCTX->directEnd();
}


void gBloom()
{
	const int BLOOM_ITERATIONS = 5;

	CD3DDevice& dx = CD3DDevice::getInstance();

	D3DSURFACE_DESC desc;
	RGET_S_TEX(RT_BLOOM)->getObject()->GetLevelDesc( 0, &desc );

	float tU = 1.0f / desc.Width;
	float tV = 1.0f / desc.Height;
	
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

	for( int i = 0; i < BLOOM_ITERATIONS; ++i ) {
		float offsetU = (i*1.05f + 0.5f) * tU;
		float offsetV = (i*1.05f + 0.5f) * tV;
		gSampleOffsets[0].set( -offsetU,  offsetV );
		gSampleOffsets[1].set( -offsetU, -offsetV );
		gSampleOffsets[2].set(  offsetU,  offsetV );
		gSampleOffsets[3].set(  offsetU, -offsetV );
		const char* texSrc;
		const char* texDst;
		texSrc = (i==0) ? RT_BRIGHT_PASS : RT_BLOOM_TMP[(i+1)%NUM_BLOOM_TMP_TEXTURES];
		texDst = (i==BLOOM_ITERATIONS-1) ? RT_BLOOM : RT_BLOOM_TMP[i%NUM_BLOOM_TMP_TEXTURES];
		dx.setRenderTarget( RGET_S_SURF(texDst) );
		dx.getStateManager().SetTexture( 0, RGET_S_TEX(texSrc)->getObject() );
		G_RENDERCTX->directBegin();
		G_RENDERCTX->directRender( *gQuadIterativeBloom );
		G_RENDERCTX->directEnd();
	}

	
	//dx.getDevice().StretchRect( RGET_S_SURF(RT_BRIGHT_PASS)->getObject(), 0, RGET_S_SURF(RT_BLOOM)->getObject(), 0, D3DTEXF_NONE );
}



static void gRender()
{
	CD3DDevice& dx = CD3DDevice::getInstance();

	// render scene to shadow map
	gShadowRender();

	// render scene to backbuffer and copy to scene rendertarget
	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0x00000000, 1.0f, 0L );
	gCamera.setOntoRenderContext();
	G_RENDERCTX->applyGlobalEffect();
	G_RENDERCTX->attach( *gMeshNormal );
	G_RENDERCTX->attach( *gMeshGround );
	G_RENDERCTX->attach( *gMeshLight );
	G_RENDERCTX->attach( *gMeshSkybox );
	G_RENDERCTX->perform();

	dx.setZStencil( NULL );
	
	dx.getDevice().StretchRect( dx.getBackBuffer(), 0, RGET_S_SURF(RT_SCENE)->getObject(), 0, D3DTEXF_NONE );

    // create a scaled down copy of the scene
	gDownscaleHDR4x();
	
	// measure luminance
	gMeasureLuminance();
	
	// calculate the current luminance adaptation level
	gCalculateAdaptation();

	// bright pass filter
	gBrightPass();

	// bloom
	gBloom();
	
	// composition to the LDR back buffer: tone mapping, blue shift, bloom.
	dx.setDefaultRenderTarget();

	dx.getStateManager().SetTexture( 0, RGET_S_TEX(RT_SCENE)->getObject() );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetTexture( 1, RGET_S_TEX(RT_LUM[gCurrLuminanceIndex])->getObject() );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	dx.getStateManager().SetTexture( 2, RGET_S_TEX(RT_BLOOM)->getObject() );
	dx.getStateManager().SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	dx.getStateManager().SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	G_RENDERCTX->directBegin();
	G_RENDERCTX->directRender( *gQuadFinalScenePass );
	G_RENDERCTX->directEnd();
	
	dx.setDefaultZStencil();

	//dx.getDevice().StretchRect(
	//	RGET_S_SURF(RT_BRIGHT_PASS)->getObject(), 0,
	//	dx.getBackBuffer(), 0, D3DTEXF_NONE );

	/*
	D3DVIEWPORT9 vp;
	vp.X = 300;
	vp.Y = 5;
	vp.Width = 64;
	vp.Height = 64;
	vp.MinZ = 0;
	vp.MaxZ = 1;
	dx.getDevice().SetViewport( &vp );
	dx.clearTargets( true, true, false, 0, 1.0f, 0L );
	gMesh->render();
	G_RENDERCTX->perform();

	vp.X = 0; vp.Y = 0;
	vp.Width = dx.getBackBufferWidth();
	vp.Height = dx.getBackBufferHeight();
	dx.getDevice().SetViewport( &vp );
	*/
}



/// Main loop code.
void CDemo::perform()
{
	char buf[100];

	G_INPUTCTX->perform();
	
	double t = CSystemTimer::getInstance().getTimeS();
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	gTimeParam = float(t);
	gDeltaTimeParam = dt;

	gHDRMiddleGray = 0.045f * (1<<gUISldMiddleGray->getValue());
	sprintf( buf, "%g", gHDRMiddleGray );
	gUILabMiddleGray->setText( buf );


	CD3DDevice& dx = CD3DDevice::getInstance();

	// FPS
	sprintf( buf, "fps: %6.1f", dx.getStats().getFPS() );
	gUIFPS->setText( buf );

	// camera
	SMatrix4x4& mc = gCamera.mWorldMat;
	D3DXMatrixRotationYawPitchRoll( &mc, gCamYaw, gCamPitch, 0.0f );
	mc.getOrigin() = gSceneCenter;
	mc.getOrigin().y *= 0.7f;
	mc.getOrigin() -= mc.getAxisZ() * gCamDist;
	if( mc.getOrigin().y < gSceneRadius*0.01f )
		mc.getOrigin().y = gSceneRadius*0.01f;
	const float camnear = gCamDist * 0.1f;
	const float camfar = gCamDist + gSceneRadius*3.0f;
	const float camfov = D3DX_PI/4;
	gCamera.setProjectionParams( camfov, dx.getBackBufferAspect(), camnear, camfar );

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

	safeDelete( gMeshNormal );
	safeDelete( gMeshGround );
	safeDelete( gMeshCaster );
	safeDelete( gMeshLight );

	safeDelete( gQuadDownsampleBB );
	safeDelete( gQuadSampleAvgLum );
	safeDelete( gQuadResampleAvgLum );
	safeDelete( gQuadResampleAvgLumExp );
	safeDelete( gQuadCalcAdaptedLum );
	safeDelete( gQuadBrightPass );
	safeDelete( gQuadIterativeBloom );
	safeDelete( gQuadFinalScenePass );
	
	safeDelete( gMeshSkybox );
}
