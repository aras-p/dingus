#include "stdafx.h"
#include "Shadows.h"
#include "ShadowBufferRTManager.h"
#include <dingus/gfx/GfxUtils.h>
#include "Hull.h"


extern	bool gUseDSTShadows;


// --------------------------------------------------------------------------

SceneEntityPtrs			gScene;
LightPtrs				gLights;


struct ObjectInfo {
	ObjectInfo( const CAABox& aabb_, SceneEntity& e ) : aabb(aabb_), entity(&e) { }

	CAABox			aabb; // world space
	SceneEntity*	entity;
};

std::vector<ObjectInfo>	gSceneReceivers; // objects in view frustum
std::vector<ObjectInfo>	gSceneCasters; // object extrusions in view frustum

CAABox			gSceneBounds, gCasterBounds, gReceiverBounds;


static SVector3 gLightDir;
static SVector3 gLightColor;
static SMatrix4x4 gShadowTexProj;
static SVector3 gInvShadowSize;

// ---------------------------------------------------------------------------

#define FLT_AS_INT(F) (*(DWORD*)&(F))
#define FLT_ALMOST_ZERO(F) ((FLT_AS_INT(F) & 0x7f800000L)==0)
#define FLT_IS_SPECIAL(F)  ((FLT_AS_INT(F) & 0x7f800000L)==0x7f800000L)


struct Frustum
{
	Frustum( const SMatrix4x4& matrix );

	bool TestSphere     ( const SVector3& pos, float radius ) const;
	//bool TestBox        ( const CAABox& box ) const;
	bool TestSweptSphere( const SVector3& pos, float radius, const SVector3& sweepDir ) const;

	SPlane		camPlanes[6];
	SVector3	pntList[8];
	int			nVertexLUT[6];
};


// Computes the point where three planes intersect. Returns whether or not the point exists.
static inline bool PlaneIntersection( SVector3* intersectPt, const SPlane& p0, const SPlane& p1, const SPlane& p2 )
{
	SVector3 n0( p0.a, p0.b, p0.c );
	SVector3 n1( p1.a, p1.b, p1.c );
	SVector3 n2( p2.a, p2.b, p2.c );

	SVector3 n1_n2, n2_n0, n0_n1;  

	D3DXVec3Cross( &n1_n2, &n1, &n2 );
	D3DXVec3Cross( &n2_n0, &n2, &n0 );
	D3DXVec3Cross( &n0_n1, &n0, &n1 );

	float cosTheta = n0.dot(n1_n2);

	if( FLT_ALMOST_ZERO(cosTheta) || FLT_IS_SPECIAL(cosTheta) )
		return false;

	float secTheta = 1.0f / cosTheta;

	n1_n2 = n1_n2 * p0.d;
	n2_n0 = n2_n0 * p1.d;
	n0_n1 = n0_n1 * p2.d;

	*intersectPt = -(n1_n2 + n2_n0 + n0_n1) * secTheta;
	return true;
}

Frustum::Frustum( const SMatrix4x4& matrix )
{
	//  build a view frustum based on the current view & projection matrices...
	SVector4 column4( matrix._14, matrix._24, matrix._34, matrix._44 );
	SVector4 column1( matrix._11, matrix._21, matrix._31, matrix._41 );
	SVector4 column2( matrix._12, matrix._22, matrix._32, matrix._42 );
	SVector4 column3( matrix._13, matrix._23, matrix._33, matrix._43 );

	SVector4 planes[6];
	planes[0] = column4 - column1;  // left
	planes[1] = column4 + column1;  // right
	planes[2] = column4 - column2;  // bottom
	planes[3] = column4 + column2;  // top
	planes[4] = column4 - column3;  // near
	planes[5] = column4 + column3;  // far

	int p;

	for (p=0; p<6; p++)  // normalize the planes
	{
		float dot = planes[p].x*planes[p].x + planes[p].y*planes[p].y + planes[p].z*planes[p].z;
		dot = 1.0f / sqrtf(dot);
		planes[p] = planes[p] * dot;
	}

	for (p=0; p<6; p++)
		camPlanes[p] = SPlane( planes[p].x, planes[p].y, planes[p].z, planes[p].w );

	//  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
	for (int i=0; i<6; i++)
		nVertexLUT[i] = ((planes[i].x<0.f)?1:0) | ((planes[i].y<0.f)?2:0) | ((planes[i].z<0.f)?4:0);

	for( int i=0; i<8; ++i ) // compute extrema
	{
		const SPlane& p0 = (i&1)?camPlanes[4] : camPlanes[5];
		const SPlane& p1 = (i&2)?camPlanes[3] : camPlanes[2];
		const SPlane& p2 = (i&4)?camPlanes[0] : camPlanes[1];

		PlaneIntersection( &pntList[i], p0, p1, p2 );
	}
}

bool Frustum::TestSphere( const SVector3& pos, float radius ) const
{
	bool inside = true;
	for( int i=0; (i<6) && inside; i++ )
		inside &= ( (D3DXPlaneDotCoord(&camPlanes[i], &pos) + radius) >= 0.0f );
	return inside;
}

//  this function tests if the projection of a bounding sphere along the light direction intersects
//  the view frustum 
bool SweptSpherePlaneIntersect( float& t0, float& t1, const SPlane& plane, const SVector3& pos, float radius, const SVector3& sweepDir )
{
	float b_dot_n = D3DXPlaneDotCoord(&plane, &pos);
	float d_dot_n = D3DXPlaneDotNormal(&plane, &sweepDir);

	if (d_dot_n == 0.f)
	{
		if (b_dot_n <= radius)
		{
			//  effectively infinity
			t0 = 0.f;
			t1 = 1e32f;
			return true;
		}
		else
			return false;
	}
	else
	{
		float tmp0 = ( radius - b_dot_n) / d_dot_n;
		float tmp1 = (-radius - b_dot_n) / d_dot_n;
		t0 = min(tmp0, tmp1);
		t1 = max(tmp0, tmp1);
		return true;
	}
}

bool Frustum::TestSweptSphere( const SVector3& pos, float radius, const SVector3& sweepDir ) const
{
	//  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
	//  for all points >0, displace sphere along the sweep direction.  if the displaced sphere
	//  is inside the frustum, return true.  else, return false
	float displacements[12];
	int cnt = 0;
	float a, b;
	bool inFrustum = false;

	for (int i=0; i<6; i++)
	{
		if (SweptSpherePlaneIntersect(a, b, camPlanes[i], pos, radius, sweepDir))
		{
			if (a>=0.f)
				displacements[cnt++] = a;
			if (b>=0.f)
				displacements[cnt++] = b;
		}
	}

	for (int i=0; i<cnt; i++)
	{
		SVector3 displPos = pos + sweepDir * displacements[i];
		float displRadius = radius * 1.1f;
		inFrustum |= TestSphere( displPos, displRadius );
	}
	return inFrustum;
}


// --------------------------------------------------------------------------

void CalculateSceneBounds()
{
	gSceneBounds.setNull();
	size_t n = gScene.size();
	for( size_t i = 0; i < n; ++i )
	{
		CAABox b = gScene[i]->getAABB();
		b.transform( gScene[i]->mWorldMat );
		gSceneBounds.extend( b );
	}
}

// Computes region-of-interest for the camera based on swept-sphere/frustum intersection.
// If the swept sphere representing the extrusion of an object's bounding
// sphere along the light direction intersects the view frustum, the object is added to
// a list of interesting shadow casters. The field of view is the minimum cone containing
//  all eligible bounding spheres.

void ComputeInterestingSceneParts( const SVector3& lightDir, const SMatrix4x4& cameraViewProj )
{
	bool hit = false;

	SVector3 sweepDir = lightDir;

	// camera frustum
	Frustum sceneFrustum( cameraViewProj );

	gSceneReceivers.clear();
	gSceneCasters.clear();
	gCasterBounds.setNull();
	gReceiverBounds.setNull();

	size_t n = gScene.size();
	for( size_t i = 0; i < n; ++i )
	{
		SceneEntity& obj = *gScene[i];
		CAABox aabb = obj.getAABB();
		aabb.transform( obj.mWorldMat );
		if( aabb.frustumCull( cameraViewProj ) )
		{
			SVector3 spherePos = aabb.getCenter();
			float sphereRadius = SVector3(aabb.getMax()-aabb.getMin()).length() / 2;
			if( sceneFrustum.TestSweptSphere( spherePos, sphereRadius, sweepDir ) )
			{
				hit = true;
				gSceneCasters.push_back( ObjectInfo(aabb,obj) ); // originally in view space
				gCasterBounds.extend( aabb );
			}
		}
		else
		{
			hit = true;
			gSceneCasters.push_back( ObjectInfo(aabb,obj) ); // originally in view space
			gSceneReceivers.push_back( ObjectInfo(aabb,obj) ); // originally in view space
			gCasterBounds.extend( aabb );
			gReceiverBounds.extend( aabb );
		}
	}
}


static void CalculateOrthoShadow( const CCameraEntity& cam, Light& light )
{
	SVector3 target = gCasterBounds.getCenter();
	SVector3 size = gCasterBounds.getMax() - gCasterBounds.getMin();
	float radius = size.length() * 0.5f;
	
	// figure out the light camera matrix that encloses whole scene
	light.mWorldMat.spaceFromAxisZ();
	light.mWorldMat.getOrigin() = target - light.mWorldMat.getAxisZ() * radius;
	light.setOrthoParams( radius*2, radius*2, radius*0.1f, radius*2 );
	light.setOntoRenderContext();
}

static void CalculateLisPSM( const CCameraEntity& cam, Light& light )
{
	CalculateOrthoShadow( cam, light );

	const SVector3& lightDir = light.mWorldMat.getAxisZ();
	const SVector3& viewDir = cam.mWorldMat.getAxisZ();
	double dotProd = lightDir.dot( viewDir );
	if( fabs(dotProd) >= 0.999 )
	{
		// degenerates to uniform shadow map
		return;
	}

	// calculate the hull of body B in world space
	HullFace bodyB;
	SMatrix4x4 invCamVP;
	D3DXMatrixInverse( &invCamVP, NULL, &cam.mWorldMat );
	invCamVP *= cam.getProjectionMatrix();
	D3DXMatrixInverse( &invCamVP, NULL, &invCamVP );

	CalculateFocusedLightHull( invCamVP, lightDir, gCasterBounds, bodyB );
	int zzz = bodyB.v.size();

	int i, j;
	/*
	Frustum camFrustum( cam.getProjectionMatrix() );
	std::vector<SVector3>	bodyB;
	bodyB.reserve( gSceneCasters.size()*8 + 8 );
	for( i = 0; i < 8; ++i )
		bodyB.push_back( camFrustum.pntList[i] );
	int ncasters = gSceneCasters.size();
	for( i = 0; i < ncasters; ++i )
	{
		const CAABox& aabb = gSceneCasters[i].aabb;
		for( j = 0; j < 8; ++j )
		{
			SVector3 p;
			p.x = (j&1) ? aabb.getMin().x : aabb.getMax().x;
			p.y = (j&2) ? aabb.getMin().y : aabb.getMax().y;
			p.z = (j&4) ? aabb.getMin().z : aabb.getMax().z;
			bodyB.push_back( p );
		}
	}
	*/
	
	// calculate basis of light space projection
	SVector3 ly = -lightDir;
	SVector3 lx = ly.cross( viewDir ).getNormalized();
	SVector3 lz = lx.cross( ly );
	SMatrix4x4 lightW;
	lightW.identify();
	lightW.getAxisX() = lx;
	lightW.getAxisY() = ly;
	lightW.getAxisZ() = lz;

	SMatrix4x4 lightV;
	D3DXMatrixInverse( &lightV, NULL, &lightW );

	// rotate bound body points from world into light projection space and calculate AABB there
	D3DXVec3TransformCoordArray( &bodyB.v[0], sizeof(SVector3), &bodyB.v[0], sizeof(SVector3), &lightV, bodyB.v.size() );
	CAABox bodyLBounds;
	bodyLBounds.setNull();
	for( i = 0; i < bodyB.v.size(); ++i )
		bodyLBounds.extend( bodyB.v[i] );
	
	// calculate free parameter N
	double sinGamma = sqrt( 1.0-dotProd*dotProd );
	const double n = ( cam.getZNear() + sqrt(cam.getZFar()*cam.getZNear()) ) / sinGamma;

	// origin in this light space: looking at center of bounds, from distance n
	SVector3 lightSpaceO = bodyLBounds.getCenter();
	lightSpaceO.z = bodyLBounds.getMin().z - n;

	// go through bound points in light space, and compute projected bound
	float maxx = 0.0f, maxy = 0.0f, maxz = 0.0f;
	for( i = 0; i < bodyB.v.size(); ++i )
	{
		SVector3 tmp = bodyB.v[i] - lightSpaceO;
		assert( tmp.z > 0.0f );
		maxx = max( maxx, fabsf(tmp.x / tmp.z) );
		maxy = max( maxy, fabsf(tmp.y / tmp.z) );
		maxz = max( maxz, tmp.z );
	}

	SVector3 lpos;
	D3DXVec3TransformCoord( &lpos, &lightSpaceO, &lightW );
	lightW.getOrigin() = lpos;

	SMatrix4x4 lightProj;
	D3DXMatrixPerspectiveLH( &lightProj, 2.0f*maxx*n, 2.0f*maxy*n, n, maxz );

	SMatrix4x4 lsPermute, lsOrtho;
	lsPermute._11 = 1.f; lsPermute._12 = 0.f; lsPermute._13 = 0.f; lsPermute._14 = 0.f;
	lsPermute._21 = 0.f; lsPermute._22 = 0.f; lsPermute._23 =-1.f; lsPermute._24 = 0.f;
	lsPermute._31 = 0.f; lsPermute._32 = 1.f; lsPermute._33 = 0.f; lsPermute._34 = 0.f;
	lsPermute._41 = 0.f; lsPermute._42 = -0.5f; lsPermute._43 = 1.5f; lsPermute._44 = 1.f;

	D3DXMatrixOrthoLH( &lsOrtho, 2.f, 1.f, 0.5f, 2.5f );
	lsPermute *= lsOrtho;
	lightProj *= lsPermute;

	G_RENDERCTX->getCamera().setCameraMatrix( lightW );

	SMatrix4x4 lightFinal = G_RENDERCTX->getCamera().getViewMatrix() * lightProj;

	// unit cube clipping
	/*
	{
		// receiver hull
		std::vector<SVector3> receiverPts;
		receiverPts.reserve( gSceneReceivers.size() * 8 );
		int nreceivers = gSceneReceivers.size();
		for( i = 0; i < nreceivers; ++i )
		{
			const CAABox& aabb = gSceneReceivers[i].aabb;
			for( j = 0; j < 8; ++j )
			{
				SVector3 p;
				p.x = (j&1) ? aabb.getMin().x : aabb.getMax().x;
				p.y = (j&2) ? aabb.getMin().y : aabb.getMax().y;
				p.z = (j&4) ? aabb.getMin().z : aabb.getMax().z;
				receiverPts.push_back( p );
			}
		}

		// transform to light post-perspective space
		D3DXVec3TransformCoordArray( &receiverPts[0], sizeof(SVector3), &receiverPts[0], sizeof(SVector3), &lightFinal, receiverPts.size() );
		CAABox recvBounds;
		recvBounds.setNull();
		for( i = 0; i < receiverPts.size(); ++i )
			recvBounds.extend( receiverPts[i] );
		
		recvBounds.getMax().x = min( 1.f, recvBounds.getMax().x );
		recvBounds.getMin().x = max(-1.f, recvBounds.getMin().x );
		recvBounds.getMax().y = min( 1.f, recvBounds.getMax().y );
		recvBounds.getMin().y = max(-1.f, recvBounds.getMin().y );
		float boxWidth = recvBounds.getMax().x - recvBounds.getMin().x;
		float boxHeight = recvBounds.getMax().y - recvBounds.getMin().y;

		if( !FLT_ALMOST_ZERO(boxWidth) && !FLT_ALMOST_ZERO(boxHeight) )
		{
			float boxX = ( recvBounds.getMax().x + recvBounds.getMin().x ) * 0.5f;
			float boxY = ( recvBounds.getMax().y + recvBounds.getMin().y ) * 0.5f;

			SMatrix4x4 clipMatrix(
				2.f/boxWidth,  0.f, 0.f, 0.f,
				0.f, 2.f/boxHeight, 0.f, 0.f,
				0.f,           0.f, 1.f, 0.f,
				-2.f*boxX/boxWidth, -2.f*boxY/boxHeight, 0.f, 1.f );
			lightProj *= clipMatrix;
		}
	}
	*/

	G_RENDERCTX->getCamera().setProjectionMatrix( lightProj );
}


void Light::PrepareAndSetOntoRenderContext( const CCameraEntity& cam, bool lispsm )
{
	SMatrix4x4 viewProj;
	D3DXMatrixInverse( &viewProj, NULL, &cam.mWorldMat );
	viewProj *= cam.getProjectionMatrix();
	ComputeInterestingSceneParts( mWorldMat.getAxisZ(), viewProj );

	if( lispsm )
		CalculateLisPSM( cam, *this );
	else
		CalculateOrthoShadow( cam, *this );

	m_DebugViewProjMatrix = G_RENDERCTX->getCamera().getViewProjMatrix();

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

	if( name=="Teapot" || name=="Torus" || name=="Table" )
		m_CanAnimate = true;

	for( int i = 0; i < RMCOUNT; ++i ) {
		CRenderableMesh* rr = new CRenderableMesh( *mMesh, CRenderableMesh::ALL_GROUPS, &mWorldMat.getOrigin(), 0 );
		mRenderMeshes[i] = rr;
		rr->getParams().setEffect( *RGET_FX(FX_NAMES[i]) );
		addMatricesToParams( rr->getParams() );
	}

	CEffectParams& ep = mRenderMeshes[RM_NORMAL]->getParams();
	ep.addVector3Ref( "vLightDir", gLightDir );
	ep.addVector3Ref( "vLightColor", gLightColor );
	ep.addMatrix4x4Ref( "mShadowProj", gShadowTexProj );
	ep.addVector3Ref( "vInvShadowSize", gInvShadowSize );
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


void RenderSceneWithShadows( CCameraEntity& camera, CCameraEntity& actualCamera, float shadowQuality, bool lispsm )
{
	gShadowRTManager->BeginFrame();

	size_t i, j;
	size_t nsceneobjs = gScene.size();
	size_t nlights = gLights.size();

	// Render the shadow buffers
	CD3DDevice& dx = CD3DDevice::getInstance();
	
	if( gUseDSTShadows )
	{
		const float kDepthBias = 0.0005f;
		const float kSlopeBias = 2.0f;
		dx.getStateManager().SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		dx.getStateManager().SetRenderState( D3DRS_DEPTHBIAS, *(DWORD*)&kDepthBias );
		dx.getStateManager().SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, *(DWORD*)&kSlopeBias );
	}

	for( i = 0; i < nlights; ++i )
	{
		Light& light = *gLights[i];
		light.m_RTSize = 1 << int(shadowQuality);
		bool ok = gShadowRTManager->RequestBuffer(
			light.m_RTSize,
			&light.m_RTTexture,
			&light.m_RTSurface );
		// set render target to this SB
		if( gUseDSTShadows )
		{
			dx.setRenderTarget( gShadowRTManager->FindDepthBuffer( light.m_RTSize ) );
			dx.setZStencil( light.m_RTSurface );
			dx.clearTargets( false, true, false, 0xFFffffff, 1.0f );
		}
		else
		{
			dx.setRenderTarget( light.m_RTSurface );
			dx.setZStencil( gShadowRTManager->FindDepthBuffer( light.m_RTSize ) );
			dx.clearTargets( true, true, false, 0xFFffffff, 1.0f );
		}
		// calculate and set SB camera matrices
		light.PrepareAndSetOntoRenderContext( camera, lispsm );
		G_RENDERCTX->applyGlobalEffect();
		// render all casters
		G_RENDERCTX->directBegin();
		for( j = 0; j < nsceneobjs; ++j )
			gScene[j]->render( RM_CASTER, true, true );
		G_RENDERCTX->directEnd();
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
	actualCamera.setOntoRenderContext();
	G_RENDERCTX->applyGlobalEffect();

	// lay down depth for all visible objects
	G_RENDERCTX->directBegin();
	for( i = 0; i < nsceneobjs; ++i )
	{
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

	// render all objects with shadows and lights
	for( i = 0; i < nsceneobjs; ++i )
	{
		SceneEntity& obj = *gScene[i];
		// additively render all lights on this object
		for( size_t j = 0; j < nlights; ++j )
		{
			Light& light = *gLights[j];

			// set the SB
			dx.getStateManager().SetTexture( kShaderShadowTextureIndex, light.m_RTTexture->getObject() );

			// set light constants
			gLightDir = light.mWorldMat.getAxisZ();
			gLightColor = light.m_Color;
			gShadowTexProj = light.m_TextureProjMatrix;
			gInvShadowSize.x = 1.0f / light.m_RTSize;
			gInvShadowSize.y = 1.0f / light.m_RTSize;

			obj.render( RM_NORMAL, false, true );
		}
	}
	
	dx.getStateManager().SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	dx.getStateManager().SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	dx.getStateManager().SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	G_RENDERCTX->directEnd();
}

// --------------------------------------------------------------------------
