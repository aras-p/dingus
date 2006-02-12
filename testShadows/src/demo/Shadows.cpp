#include "stdafx.h"
#include "Shadows.h"
#include "ShadowBufferRTManager.h"
#include <dingus/gfx/GfxUtils.h>


extern	bool gUseDSTShadows;


// --------------------------------------------------------------------------

SceneEntityPtrs			gScene;
ShadowBufferPtrs		gShadowBuffers;
LightPtrs				gLights;


static SVector3 gLightPos;
static SVector3 gLightDir;
static SVector3 gLightColor;
static float gLightCosAngle;
static SMatrix4x4 gShadowTexProj;


// --------------------------------------------------------------------------


void ShadowBuffer::SetOntoRenderContext()
{
	// figure out the camera matrix
	SMatrix4x4 matCam;
	matCam.identify();
	matCam.getAxisZ() = m_LSpaceCone.axis;
	matCam.spaceFromAxisZ();
	matCam.getOrigin() = m_Light->mWorldMat.getOrigin();
	G_RENDERCTX->getCamera().setCameraMatrix( matCam );
	
	// figure out the projection matrix
	SMatrix4x4 matProj;
	float fov = acosf( m_LSpaceCone.cosAngle ) * 2.0f;
	D3DXMatrixPerspectiveFovLH( &matProj, fov, 1.0f, m_Light->getZNear(), m_Light->getZFar() );
	G_RENDERCTX->getCamera().setProjectionMatrix( matProj );

	// calculate the rest of matrices for shadow projections
	gfx::textureProjectionWorld( G_RENDERCTX->getCamera().getViewProjMatrix(), float(m_RTSize), float(m_RTSize), m_TextureProjMatrix );
}


// --------------------------------------------------------------------------

const char* FX_NAMES[RMCOUNT] = {
	"object",
	"zfill",
	"caster",
};

SceneEntity::SceneEntity( const std::string& name )
:	mMesh(0)
,	m_CanAnimate(false)
{
	assert( !name.empty() );
	mMesh = RGET_MESH(name);

	if( name=="Teapot" || name=="Torus" )
		m_CanAnimate = true;

	const CAABox& aabb = mMesh->getTotalAABB();
	// not really optimal, but oh well!
	mRadius = max( aabb.getMax().length(), aabb.getMin().length() );

	for( int i = 0; i < RMCOUNT; ++i ) {
		CRenderableMesh* rr = new CRenderableMesh( *mMesh, CRenderableMesh::ALL_GROUPS, &mWorldMat.getOrigin(), 0 );
		mRenderMeshes[i] = rr;
		rr->getParams().setEffect( *RGET_FX(FX_NAMES[i]) );
		addMatricesToParams( rr->getParams() );
	}

	CEffectParams& ep = mRenderMeshes[RM_NORMAL]->getParams();
	ep.addVector3Ref( "vLightPos", gLightPos );
	ep.addVector3Ref( "vLightDir", gLightDir );
	ep.addVector3Ref( "vLightColor", gLightColor );
	ep.addFloatRef( "fLightCosAngle", &gLightCosAngle );
	ep.addMatrix4x4Ref( "mShadowProj", gShadowTexProj );
	ep.addVector3Ref( "vColor", m_Color );
}

SceneEntity::~SceneEntity()
{
	for( int i = 0; i < RMCOUNT; ++i )
		delete mRenderMeshes[i];
}

void SceneEntity::render( eRenderMode renderMode, bool updateWVP, bool direct )
{
	assert( mRenderMeshes[renderMode] );
	
	if( updateWVP )
		updateWVPMatrices();

	if( direct )
		G_RENDERCTX->directRender( *mRenderMeshes[renderMode] );
	else
		G_RENDERCTX->attach( *mRenderMeshes[renderMode] );
}

// --------------------------------------------------------------------------

struct ObjectRenderData
{
	bool		visibleToCamera;
	ViewCone	camSpaceCone;
	float		desiredPixelDensity;
};


const float kWidestViewConeCosAngle = 0.5f; // full cone is 120 degrees


static void OptimizeShadowConeForSpotLight( ShadowBuffer* sb )
{
#if !_DEBUG
	// don't do the optimization in debug mode - makes error checks harder

	// for spot light, it makes no sense to use wider cone than the spot light itself
	const float kWiderABit = 0.01f;
	if( sb->m_LSpaceCone.cosAngle + kWiderABit < sb->m_Light->getFOVConeCosAngle() )
	{
		sb->m_LSpaceCone.axis = sb->m_Light->getViewCone().axis;
		sb->m_LSpaceCone.cosAngle = sb->m_Light->getFOVConeCosAngle() - kWiderABit;
		sb->m_LSpaceCone.UpdateAngle();
	}
#endif
}


#if _DEBUG
static void CheckIfSBEnclosesReceivers( const ShadowBuffer* sb )
{
	size_t nobjs = sb->m_Entities.size();
	const SVector3 lightPos = sb->m_Light->mWorldMat.getOrigin();
	for( size_t i = 0; i < nobjs; ++i )
	{
		const SceneEntity& obj = *sb->m_Entities[i];
		SVector3 lightRelPos = obj.mWorldMat.getOrigin() - lightPos;
		ViewCone objLightSpaceCone;
		ConeMake( lightRelPos, obj.getRadius(), objLightSpaceCone );
		bool encloses = DoesCone2EncloseCone1( objLightSpaceCone, sb->m_LSpaceCone );
		assert( encloses );
	}
}
#endif



void RenderSceneWithShadows( CCameraEntity& camera, float shadowQuality )
{
	camera.updateViewConeAngleFull();
	camera.updateViewCone();

	gShadowRTManager->BeginFrame();

	size_t i, j;
	size_t nsceneobjs = gScene.size();
	size_t nlights = gLights.size();

	static std::vector<ObjectRenderData>	objRenderData;
	objRenderData.resize( nsceneobjs );

	// clear shadow buffers
	stl_utils::wipe( gShadowBuffers );

	// check which objects are visible, calculate camera space cones and
	// desired pixel densities
	for( i = 0; i < nsceneobjs; ++i )
	{
		SceneEntity& obj = *gScene[i];
		obj.m_ShadowBuffers.clear();

		SVector3 eyeRelPos = obj.mWorldMat.getOrigin() - camera.mWorldMat.getOrigin();
		bool vis = camera.frustumCullRelPos( eyeRelPos, obj.getRadius() );
		objRenderData[i].visibleToCamera = vis;
		if( vis )
		{
			ConeMake( eyeRelPos, obj.getRadius(), objRenderData[i].camSpaceCone );
			float dist = camera.distanceToRelPos( eyeRelPos );
			if( dist < camera.getZNear() )
				dist = camera.getZNear();
			objRenderData[i].desiredPixelDensity = shadowQuality / dist;
		}
	}

	// for all lights
	for( i = 0; i < nlights; ++i )
	{
		Light& light = *gLights[i];
		// clear lists of this light
		light.m_Entities.clear();
		light.m_ShadowBuffers.clear();

		// for all objects
		for( j = 0; j < nsceneobjs; ++j )
		{
			SceneEntity& obj = *gScene[j];

			// check if object is affected by this light
			// for now just light frustum check, no occlusion culling
			SVector3 lightRelPos = obj.mWorldMat.getOrigin() - light.mWorldMat.getOrigin();
			bool vis = light.frustumCullRelPos( lightRelPos, obj.getRadius() );
			if( !vis )
				continue;
			// add to lights' list
			light.m_Entities.push_back( &obj );

			// make light space cone for the object
			ViewCone objLightSpaceCone;
			ConeMake( lightRelPos, obj.getRadius(), objLightSpaceCone );

			const ObjectRenderData& objData = objRenderData[j];
			if( !objData.visibleToCamera )
				continue;

			// we know that the object is a receiver for this light
			bool found = false;

			float objDistToLight = light.distanceToRelPos( lightRelPos );
			if( objDistToLight < light.getZNear() )
				objDistToLight = light.getZNear();

			// Try to find an existing shadowbuffer that encloses the object
			// and has a high enough density
			size_t nLightSBs = light.m_ShadowBuffers.size();
			for( size_t sb = 0; sb < nLightSBs; ++sb )
			{
				ShadowBuffer& shadowBuffer = *light.m_ShadowBuffers[sb];
				// if the object cone is enclosed by the shadowbuffer cone
				// TBD: maybe enough to test SBCone-ObjSphere intersection?
				if( DoesCone2EncloseCone1( objLightSpaceCone, shadowBuffer.m_LSpaceCone ) )
				{
					float pixelsPerMeter = shadowBuffer.m_PixelDensity / objDistToLight;
					if( pixelsPerMeter >= objData.desiredPixelDensity )
					{
#if _DEBUG
						assert( std::find(shadowBuffer.m_Entities.begin(), shadowBuffer.m_Entities.end(), &obj) == shadowBuffer.m_Entities.end() );
						shadowBuffer.m_Entities.push_back( &obj );
						CheckIfSBEnclosesReceivers( &shadowBuffer );
#endif
						obj.m_ShadowBuffers.push_back( &shadowBuffer );
						found = true;
						break;
					}
				}
			}

			if( !found )
			{
				// None of the existing buffers is good enough. Look for one we can grow slightly.
				// There are often a number of them, so try to find the best
				// (the one that ends up having the smallest base-cone angle).
				ShadowBuffer* bestSB = NULL;
				ViewCone bestCone;
				bestCone.cosAngle = -1.0f; // smaller angle - larger cosine
				float bestPixelDensity = -1.0f;
				float desiredPixelDensity = objData.desiredPixelDensity * objDistToLight;
				
				size_t nLightSBs = light.m_ShadowBuffers.size();
				for( size_t sb = 0; sb < nLightSBs; ++sb )
				{
					ShadowBuffer& shadowBuffer = *light.m_ShadowBuffers[sb];
					// Find the union between the SB's cone and the object's light-space cone
					ViewCone unionCone;
					eConeUnionResult res = ConeUnion( &unionCone, shadowBuffer.m_LSpaceCone, objLightSpaceCone, true );
					if( res == CUR_NOBOUND )
						continue;
					if( unionCone.cosAngle < kWidestViewConeCosAngle )
						continue;
					if( unionCone.cosAngle < bestCone.cosAngle )
						continue;
					float newPixelDensity = max( shadowBuffer.m_PixelDensity, desiredPixelDensity );
					float textureSize = ConeSizeOfTexture( unionCone, newPixelDensity );
					if( textureSize > kMaxShadowRTSize )
						continue;

					bestSB = &shadowBuffer;
					bestCone = unionCone;
					bestPixelDensity = newPixelDensity;
					found = true;
				}

				if( found )
				{
					obj.m_ShadowBuffers.push_back( bestSB );
					bestSB->m_LSpaceCone = bestCone;
					bestSB->m_PixelDensity = bestPixelDensity;
					OptimizeShadowConeForSpotLight( bestSB );
#if _DEBUG
					assert( std::find(bestSB->m_Entities.begin(), bestSB->m_Entities.end(), &obj) == bestSB->m_Entities.end() );
					bestSB->m_Entities.push_back( &obj );
					CheckIfSBEnclosesReceivers( bestSB );
#endif
				}
			}

			if( !found )
			{
				// Still nothing acceptable! OK, just make a new SB then
				ShadowBuffer* shadowBuffer = new ShadowBuffer();
				shadowBuffer->m_LSpaceCone = objLightSpaceCone;
				shadowBuffer->m_PixelDensity = objData.desiredPixelDensity * objDistToLight;
				shadowBuffer->m_Light = &light;
				OptimizeShadowConeForSpotLight( shadowBuffer );
				gShadowBuffers.push_back( shadowBuffer );
#if _DEBUG
				assert( std::find(shadowBuffer->m_Entities.begin(), shadowBuffer->m_Entities.end(), &obj) == shadowBuffer->m_Entities.end() );
				shadowBuffer->m_Entities.push_back( &obj );
				CheckIfSBEnclosesReceivers( shadowBuffer );
#endif
				obj.m_ShadowBuffers.push_back( shadowBuffer );
				light.m_ShadowBuffers.push_back( shadowBuffer );
			}
		}

		// now all the receivers in this light have SBs allocated, so
		// get the real shadow buffer RTs and whatnot
		size_t nLightSBs = light.m_ShadowBuffers.size();
		for( size_t sb = 0; sb < nLightSBs; ++sb )
		{
			ShadowBuffer& shadowBuffer = *light.m_ShadowBuffers[sb];
			shadowBuffer.m_RTSize = (int)ConeSizeOfTexture( shadowBuffer.m_LSpaceCone, shadowBuffer.m_PixelDensity );
			bool ok = gShadowRTManager->RequestBuffer(
				shadowBuffer.m_RTSize,
				&shadowBuffer.m_RTTexture,
				&shadowBuffer.m_RTSurface );
			// TBD: if failed, kill the SB
		}

		// Find which objects cast shadows into the SBs
		size_t nLightObjects = light.m_Entities.size();
		nLightSBs = light.m_ShadowBuffers.size();
		for( j = 0; j < nLightObjects; ++j )
		{
			SceneEntity& obj = *light.m_Entities[j];
			SVector3 lightRelPos = obj.mWorldMat.getOrigin() - light.mWorldMat.getOrigin();
			for( size_t sb = 0; sb < nLightSBs; ++sb )
			{
				ShadowBuffer& shadowBuffer = *light.m_ShadowBuffers[sb];
				if( shadowBuffer.m_LSpaceCone.IntersectsSphere( lightRelPos, obj.getRadius() ) )
				{
					shadowBuffer.m_Casters.push_back( &obj );
				}
			}
		}
	}

	// Now that all the setup is done, actually render the shadow buffers
	CD3DDevice& dx = CD3DDevice::getInstance();
	
	if( gUseDSTShadows )
	{
		const float kDepthBias = 0.0001f;
		const float kSlopeBias = 2.0f;
		dx.getStateManager().SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		dx.getStateManager().SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)&kDepthBias );
		dx.getStateManager().SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&kSlopeBias );
	}

	for( i = 0; i < nlights; ++i )
	{
		Light& light = *gLights[i];
		size_t nLightSBs = light.m_ShadowBuffers.size();
		for( size_t sb = 0; sb < nLightSBs; ++sb )
		{
			ShadowBuffer& shadowBuffer = *light.m_ShadowBuffers[sb];
			// set render target to this SB
			if( gUseDSTShadows )
			{
				dx.setRenderTarget( gShadowRTManager->FindDepthBuffer( shadowBuffer.m_RTSize ) );
				dx.setZStencil( shadowBuffer.m_RTSurface );
				dx.clearTargets( false, true, false, 0xFFffffff, 1.0f );
			}
			else
			{
				dx.setRenderTarget( shadowBuffer.m_RTSurface );
				dx.setZStencil( gShadowRTManager->FindDepthBuffer( shadowBuffer.m_RTSize ) );
				dx.clearTargets( true, true, false, 0xFFffffff, 1.0f );
			}
			// calculate and set SB camera matrices
			shadowBuffer.SetOntoRenderContext();
			G_RENDERCTX->applyGlobalEffect();
			// render all casters
			size_t nobjs = shadowBuffer.m_Casters.size();
			G_RENDERCTX->directBegin();
			for( size_t o = 0; o < nobjs; ++o )
				shadowBuffer.m_Casters[o]->render( RM_CASTER, true, true );
			G_RENDERCTX->directEnd();
		}
	}

	if( gUseDSTShadows )
	{
		dx.getStateManager().SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA );
		const float temp = 0.0f;
		dx.getStateManager().SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)&temp );
		dx.getStateManager().SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&temp );
	}

	// Render the main camera with all the shadows and lights and whatever
	dx.setDefaultRenderTarget();
	dx.setDefaultZStencil();
	dx.clearTargets( true, true, false, 0x606060, 1.0f );
	camera.setOntoRenderContext();
	G_RENDERCTX->applyGlobalEffect();

	// lay down depth for all visible objects
	G_RENDERCTX->directBegin();
	for( i = 0; i < nsceneobjs; ++i )
	{
		if( !objRenderData[i].visibleToCamera )
			continue;
		gScene[i]->render( RM_ZFILL, true, true );
	}

	// additive lighting
	dx.getStateManager().SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	dx.getStateManager().SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	dx.getStateManager().SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	dx.getStateManager().SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	// shadow map sampling
	if( gUseDSTShadows )
	{
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	}
	else
	{
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		dx.getStateManager().SetSamplerState( kShaderShadowTextureIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
	}

	// render all visible objects with shadows and lights
	for( i = 0; i < nsceneobjs; ++i )
	{
		if( !objRenderData[i].visibleToCamera )
			continue;

		SceneEntity& obj = *gScene[i];
		// additively render all lights on this object
		size_t nObjSBs = obj.m_ShadowBuffers.size();
		assert( nObjSBs <= nlights );
		for( size_t sb = 0; sb < nObjSBs; ++sb )
		{
			ShadowBuffer& shadowBuffer = *obj.m_ShadowBuffers[sb];
			Light& light = *shadowBuffer.m_Light;

			// set the SB
			dx.getStateManager().SetTexture( kShaderShadowTextureIndex, shadowBuffer.m_RTTexture->getObject() );

			// set light constants
			gLightPos = light.mWorldMat.getOrigin();
			gLightDir = light.mWorldMat.getAxisZ();
			gLightColor = light.m_Color;
			gLightCosAngle = light.getViewCone().cosAngle;
			gShadowTexProj = shadowBuffer.m_TextureProjMatrix;

			obj.render( RM_NORMAL, false, true );
		}
	}
	
	dx.getStateManager().SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dx.getStateManager().SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	G_RENDERCTX->directEnd();
}

// --------------------------------------------------------------------------
