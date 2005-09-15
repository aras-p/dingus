#include "stdafx.h"
#include "PointsMesh.h"
#include "LevelMesh.h"
#include "../game/GameMap.h"
#include "../game/GameColors.h"

#include <dingus/resource/MeshCreator.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/Vertices.h>

// --------------------------------------------------------------------------

class CMapPointsMeshCreator : public IMeshCreator {
public:
	typedef SVertexXyzNormalDiffuse	TVertex;
public:
	CMapPointsMeshCreator( const CGameMap& gameMap, const CLevelMesh& levelMesh )
		: mGameMap( &gameMap ), mLevelMesh( &levelMesh )
	{
		const int PARTS_PER_POINT = 200;
		int n = gameMap.getPointCount();
		mParticleCount = n * PARTS_PER_POINT;
		mParticles = new SVector3[mParticleCount];
		mParticleColor = new D3DCOLOR[mParticleCount];
		for( int i = 0; i < n; ++i ) {
			const CGameMap::SPoint& p = gameMap.getPoint(i);
			float y = -gameMap.getCell(p.x,p.y).height;
			D3DCOLOR color = (p.type == PT_AZN) ? gColors.ptAZN.tone.c : gColors.ptHoshimi.tone.c;
			for( int j = 0; j < PARTS_PER_POINT; ++j ) {
				SVector3& pos = mParticles[i*PARTS_PER_POINT+j];
				mParticleColor[i*PARTS_PER_POINT+j] = color;
				pos.set( p.x, y, -p.y );
				float phi = gRandom.getFloat( 0.0f, D3DX_PI*2 );
				float theta = gRandom.getFloat( D3DX_PI/2 );
				float r = gRandom.getFloat( 0.1f, 1.2f );
				mParticleColor[i*PARTS_PER_POINT+j] &= 0x00ffffff;
				float size = gRandom.getFloat( 0.5f, 1.0f );
				mParticleColor[i*PARTS_PER_POINT+j] |= (int)(size*255.0f) << 24;
				pos.x += cosf( phi ) * /*cosf(theta) **/ r;
				pos.z += sinf( phi ) * /*cosf(theta) **/ r;
				pos.y += sinf( theta ) * r * 0.5f;
			}
		}
		assert( mParticleCount < 16000 );
	}
	~CMapPointsMeshCreator()
	{
		delete[] mParticles;
		delete[] mParticleColor;
	}

	virtual void createMesh( CMesh& mesh ) {
		assert( !mesh.isCreated() );

		int nverts = mParticleCount * 4;
		int ntris = mParticleCount * 2;

		CVertexFormat vformat(
			CVertexFormat::V_POSITION |
			CVertexFormat::V_NORMAL |
			CVertexFormat::COLOR_MASK
		);
		
		mesh.createResource( nverts, ntris*3, vformat, 2, *RGET_VDECL(vformat), CMesh::BUF_STATIC );

		unsigned short* pib = (unsigned short*)mesh.lockIBWrite();
		assert( pib );
		TVertex* pvb = (TVertex*)mesh.lockVBWrite();
		assert( pvb );

		for( int i = 0; i < mParticleCount; ++i ) {
			// IB
			pib[0] = i*4+0;
			pib[1] = i*4+1;
			pib[2] = i*4+2;
			pib[3] = i*4+0;
			pib[4] = i*4+2;
			pib[5] = i*4+3;
			pib += 6;
			// VB
			const SVector3& p = mParticles[i];
			D3DCOLOR color = mParticleColor[i];
			float ao = gRandom.getFloat();
			pvb[0].p = p;	pvb[0].n.x = -0.5f;	pvb[0].n.y = -0.5f;	pvb[0].n.z = ao; pvb[0].diffuse = color;
			pvb[1].p = p;	pvb[1].n.x =  0.5f;	pvb[1].n.y = -0.5f;	pvb[1].n.z = ao; pvb[1].diffuse = color;
			pvb[2].p = p;	pvb[2].n.x =  0.5f;	pvb[2].n.y =  0.5f;	pvb[2].n.z = ao; pvb[2].diffuse = color;
			pvb[3].p = p;	pvb[3].n.x = -0.5f;	pvb[3].n.y =  0.5f;	pvb[3].n.z = ao; pvb[3].diffuse = color;
			pvb += 4;
		}

		mesh.unlockIBWrite();
		mesh.unlockVBWrite();

		mesh.computeAABBs();
	}

private:
	const CGameMap*		mGameMap;
	const CLevelMesh*	mLevelMesh;
	
	int			mParticleCount;
	SVector3*	mParticles;
	D3DCOLOR*	mParticleColor;
};




// --------------------------------------------------------------------------

CPointsMesh::CPointsMesh( const CGameMap& gameMap, const CLevelMesh& levelMesh )
{
	CSharedMeshBundle::getInstance().registerMesh( RID_MESH_POINTS, *new CMapPointsMeshCreator(gameMap,levelMesh) );
	CMesh* mesh = RGET_S_MESH(RID_MESH_POINTS);
	for( int i = 0; i < RMCOUNT; ++i ) {
		mMesh[i] = NULL;
		// may not have all rendermodes
		CModelDesc* desc = RTRYGET_MDESC( (RMODE_PREFIX[i] + std::string("points")) );
		if( !desc )
			continue;
		mMesh[i] = new CRenderableMesh( *mesh, CRenderableMesh::ALL_GROUPS, NULL, desc->getRenderPriority(0) );
		desc->fillFxParams( 0, mMesh[i]->getParams() );
	}
}

CPointsMesh::~CPointsMesh()
{
	for( int i = 0; i < RMCOUNT; ++i )
		safeDelete( mMesh[i] );
}

void CPointsMesh::render( eRenderMode renderMode )
{
	if( mMesh[renderMode] )
		G_RENDERCTX->attach( *mMesh[renderMode] );
}
