#include "stdafx.h"
#include "AttackEntity.h"

#include <dingus/resource/MeshCreator.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/Vertices.h>
#include <dingus/renderer/RenderableMesh.h>

#define RID_MESH_ATKBEAM "atkbeam"
//#define RID_MESH_ATKWAVE "atkwave"
#define RID_MESH_EXPLOSION "explos"


// --------------------------------------------------------------------------

class CAttack : public boost::noncopyable {
public:
	CAttack()
	{
		mMeshAtk = new CRenderableMesh( *RGET_S_MESH(RID_MESH_ATKBEAM), CRenderableMesh::ALL_GROUPS, &mOrigin, 2 );
		{
			CEffectParams& ep = mMeshAtk->getParams();
			ep.setEffect( *RGET_FX("attackBeam") );
			ep.addVector4Ref( "vColor", mColor );
			ep.addVector3Ref( "vOrigin", mOrigin );
			ep.addVector3Ref( "vLength", mLength );
			ep.addVector3Ref( "vDirection", mDirection );
			ep.addFloatRef( "fAlpha", &mBeamAlpha );
			ep.addTexture( "tBase", *RGET_TEX("SmokeParticle") );
		}
		//mMeshWav = new CRenderableMesh( *RGET_S_MESH(RID_MESH_ATKWAVE), CRenderableMesh::ALL_GROUPS, &mOrigin, 2 );
		mMeshWav = new CRenderableMesh( *RGET_MESH("Attack"), CRenderableMesh::ALL_GROUPS, &mOrigin, 2 );
		{
			CEffectParams& ep = mMeshWav->getParams();
			ep.setEffect( *RGET_FX("attackWave") );
			ep.addVector4Ref( "vColor", mColor );
			ep.addVector3Ref( "vOrigin", mTarget );
			ep.addFloatRef( "fAlpha", &mWaveAlpha );
			//ep.addTexture( "tBase", *RGET_TEX("SmokeParticle") );
			ep.addTexture( "tBase", *RGET_TEX("PointInjection") );
		}
	}
	~CAttack()
	{
		delete mMeshAtk;
		delete mMeshWav;
	}

	void	render()
	{
		G_RENDERCTX->attach( *mMeshAtk );
		G_RENDERCTX->attach( *mMeshWav );
	}

private:
	CRenderableMesh*	mMeshAtk;
	CRenderableMesh*	mMeshWav;
public:
	SVector4	mColor;
	SVector3	mOrigin;
	SVector3	mLength;
	SVector3	mTarget;
	SVector3	mDirection;
	float		mBeamAlpha;
	float		mWaveAlpha;
};


// --------------------------------------------------------------------------

class CExplosion : public boost::noncopyable {
public:
	CExplosion()
	{
		mMesh = new CRenderableMesh( *RGET_S_MESH(RID_MESH_EXPLOSION), CRenderableMesh::ALL_GROUPS, &mOrigin, 2 );
		{
			CEffectParams& ep = mMesh->getParams();
			ep.setEffect( *RGET_FX("explosion") );
			ep.addVector3Ref( "vOrigin", mOrigin );
			ep.addFloatRef( "fAlpha", &mAlpha );
			ep.addTexture( "tBase", *RGET_TEX("Explosion") );
		}
	}
	~CExplosion()
	{
		delete mMesh;
	}

	void	render()
	{
		G_RENDERCTX->attach( *mMesh );
	}

private:
	CRenderableMesh*	mMesh;
public:
	SVector3	mOrigin;
	float		mAlpha;
};


// --------------------------------------------------------------------------

class CNimbus : public boost::noncopyable {
public:
	CNimbus()
	{
		mMesh = new CRenderableMesh( *RGET_MESH("Nimbus"), CRenderableMesh::ALL_GROUPS, &mOrigin, 2 );
		{
			CEffectParams& ep = mMesh->getParams();
			ep.setEffect( *RGET_FX("nimbus") );
			ep.addVector3Ref( "vOrigin", mOrigin );
			ep.addFloatRef( "fScale", &mScale );
			ep.addTexture( "tBase", *RGET_TEX("Nimbus") );
		}
	}
	~CNimbus()
	{
		delete mMesh;
	}

	void	render()
	{
		G_RENDERCTX->attach( *mMesh );
	}

private:
	CRenderableMesh*	mMesh;
public:
	SVector3	mOrigin;
	float		mScale;
};



// --------------------------------------------------------------------------

class CAttackBeamMeshCreator : public IMeshCreator {
public:
	typedef SVertexXyzNormal TVertex; // xyz=velocity, normal.xy=offset, normal.z=alpha
public:
	virtual void createMesh( CMesh& mesh ) {
		const int PARTS_PER_BEAM = 75;

		assert( !mesh.isCreated() );

		int nverts = PARTS_PER_BEAM * 4;
		int ntris = PARTS_PER_BEAM * 2;

		CVertexFormat vformat(
			CVertexFormat::V_POSITION |
			CVertexFormat::V_NORMAL
		);
		
		mesh.createResource( nverts, ntris*3, vformat, 2, *RGET_VDECL(vformat), CMesh::BUF_STATIC );

		unsigned short* pib = (unsigned short*)mesh.lockIBWrite();
		assert( pib );
		TVertex* pvb = (TVertex*)mesh.lockVBWrite();
		assert( pvb );

		for( int i = 0; i < PARTS_PER_BEAM; ++i ) {
			// IB
			pib[0] = i*4+0;
			pib[1] = i*4+1;
			pib[2] = i*4+2;
			pib[3] = i*4+0;
			pib[4] = i*4+2;
			pib[5] = i*4+3;
			pib += 6;
			// VB
			float speed = 0.3f * gRandom.getFloat(0.8f,1.2f);
			float phi = gRandom.getFloat( D3DX_PI*2 );
			float theta = gRandom.getFloat( -D3DX_PI/2, D3DX_PI/2 );
			SVector3 vel;
			vel.x = cosf(phi) * cosf(theta) * speed;
			vel.y = sinf(phi) * cosf(theta) * speed;
			vel.z = sinf(theta) * speed;
			float ao = gRandom.getFloat();
			pvb[0].p = vel;	pvb[0].n.x = -0.5f;	pvb[0].n.y = -0.5f;	pvb[0].n.z = ao;
			pvb[1].p = vel;	pvb[1].n.x =  0.5f;	pvb[1].n.y = -0.5f;	pvb[1].n.z = ao;
			pvb[2].p = vel;	pvb[2].n.x =  0.5f;	pvb[2].n.y =  0.5f;	pvb[2].n.z = ao;
			pvb[3].p = vel;	pvb[3].n.x = -0.5f;	pvb[3].n.y =  0.5f;	pvb[3].n.z = ao;
			pvb += 4;
		}

		mesh.unlockIBWrite();
		mesh.unlockVBWrite();

		mesh.computeAABBs();
	}
};

/*
class CAttackWaveMeshCreator : public IMeshCreator {
public:
	typedef SVertexXyzNormal TVertex; // xyz=velocity, normal.xy=offset, normal.z=alpha
public:
	virtual void createMesh( CMesh& mesh ) {
		const int PARTS_PER_WAVE = 40;

		assert( !mesh.isCreated() );

		int nverts = PARTS_PER_WAVE * 4;
		int ntris = PARTS_PER_WAVE * 2;

		CVertexFormat vformat(
			CVertexFormat::V_POSITION |
			CVertexFormat::V_NORMAL
		);
		
		mesh.createResource( nverts, ntris*3, vformat, 2, *RGET_VDECL(vformat), CMesh::BUF_STATIC );

		unsigned short* pib = (unsigned short*)mesh.lockIBWrite();
		assert( pib );
		TVertex* pvb = (TVertex*)mesh.lockVBWrite();
		assert( pvb );

		for( int i = 0; i < PARTS_PER_WAVE; ++i ) {
			// IB
			pib[0] = i*4+0;
			pib[1] = i*4+1;
			pib[2] = i*4+2;
			pib[3] = i*4+0;
			pib[4] = i*4+2;
			pib[5] = i*4+3;
			pib += 6;
			// VB
			float speed = 3.0f * gRandom.getFloat(0.2f,1.2f);
			float phi = gRandom.getFloat( D3DX_PI*2 );
			float theta = gRandom.getFloat( -D3DX_PI/5, D3DX_PI/5 );
			SVector3 vel;
			vel.x = cosf(phi) * cosf(theta) * speed;
			vel.y = sinf(theta) * speed;
			vel.z = sinf(phi) * cosf(theta) * speed;
			float ao = gRandom.getFloat();
			pvb[0].p = vel;	pvb[0].n.x = -0.5f;	pvb[0].n.y = -0.5f;	pvb[0].n.z = ao;
			pvb[1].p = vel;	pvb[1].n.x =  0.5f;	pvb[1].n.y = -0.5f;	pvb[1].n.z = ao;
			pvb[2].p = vel;	pvb[2].n.x =  0.5f;	pvb[2].n.y =  0.5f;	pvb[2].n.z = ao;
			pvb[3].p = vel;	pvb[3].n.x = -0.5f;	pvb[3].n.y =  0.5f;	pvb[3].n.z = ao;
			pvb += 4;
		}

		mesh.unlockIBWrite();
		mesh.unlockVBWrite();

		mesh.computeAABBs();
	}
};
*/

class CExplosionMeshCreator : public IMeshCreator {
public:
	typedef SVertexXyzNormal TVertex; // xyz=position, normal.xy=offset, normal.z=alpha
public:
	virtual void createMesh( CMesh& mesh ) {
		const int PARTS_PER_EXP = 25;

		assert( !mesh.isCreated() );

		int nverts = PARTS_PER_EXP * 4;
		int ntris = PARTS_PER_EXP * 2;

		CVertexFormat vformat(
			CVertexFormat::V_POSITION |
			CVertexFormat::V_NORMAL
		);
		
		mesh.createResource( nverts, ntris*3, vformat, 2, *RGET_VDECL(vformat), CMesh::BUF_STATIC );

		unsigned short* pib = (unsigned short*)mesh.lockIBWrite();
		assert( pib );
		TVertex* pvb = (TVertex*)mesh.lockVBWrite();
		assert( pvb );

		for( int i = 0; i < PARTS_PER_EXP; ++i ) {
			// IB
			pib[0] = i*4+0;
			pib[1] = i*4+1;
			pib[2] = i*4+2;
			pib[3] = i*4+0;
			pib[4] = i*4+2;
			pib[5] = i*4+3;
			pib += 6;
			// VB
			const float EXP_RADIUS = 0.6f;
			float phi = gRandom.getFloat( D3DX_PI*2 );
			float theta = gRandom.getFloat( -D3DX_PI/2, D3DX_PI/2 );
			SVector3 pos;
			pos.x = cosf(phi) * cosf(theta) * EXP_RADIUS;
			pos.y = sinf(phi) * cosf(theta) * EXP_RADIUS;
			pos.z = sinf(theta) * EXP_RADIUS;
			float ao = gRandom.getFloat( 0.5f );
			pvb[0].p = pos;	pvb[0].n.x = -0.5f;	pvb[0].n.y = -0.5f;	pvb[0].n.z = ao;
			pvb[1].p = pos;	pvb[1].n.x =  0.5f;	pvb[1].n.y = -0.5f;	pvb[1].n.z = ao;
			pvb[2].p = pos;	pvb[2].n.x =  0.5f;	pvb[2].n.y =  0.5f;	pvb[2].n.z = ao;
			pvb[3].p = pos;	pvb[3].n.x = -0.5f;	pvb[3].n.y =  0.5f;	pvb[3].n.z = ao;
			pvb += 4;
		}

		mesh.unlockIBWrite();
		mesh.unlockVBWrite();

		mesh.computeAABBs();
	}
};


// --------------------------------------------------------------------------

CAttackEntityManager::CAttackEntityManager()
{
	CSharedMeshBundle::getInstance().registerMesh( RID_MESH_ATKBEAM, *new CAttackBeamMeshCreator() );
	//CSharedMeshBundle::getInstance().registerMesh( RID_MESH_ATKWAVE, *new CAttackWaveMeshCreator() );
	CSharedMeshBundle::getInstance().registerMesh( RID_MESH_EXPLOSION, *new CExplosionMeshCreator() );
}

CAttackEntityManager::~CAttackEntityManager()
{
	// delete cached objects
	stl_utils::wipe( mAttacks );
	stl_utils::wipe( mExplosions );
	stl_utils::wipe( mNimbi );
}

void CAttackEntityManager::begin()
{
	mAttackCounter = 0;
	mExplosionCounter = 0;
	mNimbusCounter = 0;
}

void CAttackEntityManager::renderAttack( const SVector3& origin, int targx, int targy, float turnAlpha, const SVector4& color )
{
	assert( mAttackCounter <= mAttacks.size() );

	// create new attack if not enough cached ones, add to cache
	if( mAttackCounter == mAttacks.size() ) {
		CAttack* e = new CAttack();
		mAttacks.push_back( e );
	}

	// fetch pre-created entity from cache
	CAttack& atk = *mAttacks[mAttackCounter];
	++mAttackCounter;

	SVector3 destPos( targx, 0.0f, -targy );

	// set params
	atk.mColor = color;
	atk.mOrigin = origin;
	atk.mLength = destPos - origin;
	atk.mTarget = destPos;
	atk.mDirection = atk.mLength.getNormalized();
	atk.mBeamAlpha = turnAlpha;
	atk.mWaveAlpha = turnAlpha + 0.1f;

	// render
	atk.render();
}

void CAttackEntityManager::renderExplosion( const SVector3& origin, float turnAlpha )
{
	assert( mExplosionCounter <= mExplosions.size() );

	// create new explosion if not enough cached ones, add to cache
	if( mExplosionCounter == mExplosions.size() ) {
		CExplosion* e = new CExplosion();
		mExplosions.push_back( e );
	}

	// fetch pre-created entity from cache
	CExplosion& atk = *mExplosions[mExplosionCounter];
	++mExplosionCounter;

	// set params
	atk.mOrigin = origin;
	atk.mAlpha = turnAlpha;

	// render
	atk.render();
}

void CAttackEntityManager::renderNimbus( const SVector3& origin, float scale )
{
	assert( mNimbusCounter <= mNimbi.size() );

	// create new attack if not enough cached ones, add to cache
	if( mNimbusCounter == mNimbi.size() ) {
		CNimbus* e = new CNimbus();
		mNimbi.push_back( e );
	}

	// fetch pre-created entity from cache
	CNimbus& atk = *mNimbi[mNimbusCounter];
	++mNimbusCounter;

	// set params
	atk.mOrigin = origin;
	atk.mScale = scale;

	// render
	atk.render();
}
