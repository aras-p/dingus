#include "stdafx.h"
#include "LevelMesh.h"
#include "TriangleMesh.h"
#include "../game/GameMap.h"
#include "Collider.h"

#include <dingus/renderer/Renderable.h>
#include <dingus/resource/MeshCreator.h>

extern SAppSettings gAppSettings;


// --------------------------------------------------------------------------

/*
static void gOptimizeFaces( void* ib, bool ib32bit, int numFaces, int numVerts )
{
	DWORD* remap = new DWORD[numFaces];
	D3DXOptimizeFaces( ib, numFaces, numVerts, ib32bit, remap );
	if( ib32bit ) {
		unsigned int* copyIB = new unsigned int[numFaces*3];
		memcpy( copyIB, ib, numFaces*12 );
		for( int i = 0; i < numFaces; ++i ) {
			DWORD newFace = remap[i];
			for( int j = 0; j < 3; ++j ) {
				((unsigned int*)ib)[newFace*3+j] = copyIB[i*3+j];
			}
		}
		delete[] copyIB;
	} else {
		unsigned short* copyIB = new unsigned short[numFaces*3];
		memcpy( copyIB, ib, numFaces*6 );
		for( int i = 0; i < numFaces; ++i ) {
			DWORD newFace = remap[i];
			for( int j = 0; j < 3; ++j ) {
				((unsigned short*)ib)[newFace*3+j] = copyIB[i*3+j];
			}
		}
		delete[] copyIB;
	}
	delete[] remap;
}


template< class _V >
void gOptimizeVertices( _V* vb, void* ib, bool ib32bit, int numFaces, int numVerts )
{
	int i, j;

	DWORD* remap = new DWORD[numVerts];
	D3DXOptimizeVertices( ib, numFaces, numVerts, ib32bit, remap );

	// remap vertices
	_V* copyVB = new _V[numVerts];
	memcpy( copyVB, vb, numVerts*sizeof(_V) );
	for( i = 0; i < numVerts; ++i )
		vb[ remap[i] ] = copyVB[i];
	delete[] copyVB;

	// remap triangles
	if( ib32bit ) {
		unsigned int* ib32 = (unsigned int*)ib;
		for( i = 0; i < numFaces; ++i ) {
			for( j = 0; j < 3; ++j )
				ib32[i*3+j] = remap[ ib32[i*3+j] ];
		}
	} else {
		unsigned short* ib16 = (unsigned short*)ib;
		for( i = 0; i < numFaces; ++i ) {
			for( j = 0; j < 3; ++j )
				ib16[i*3+j] = (unsigned short)remap[ ib16[i*3+j] ];
		}
	}
	delete[] remap;
}
*/

// --------------------------------------------------------------------------

class CGameMapMeshData : public boost::noncopyable {
public:
	CGameMapMeshData( FILE* f )
	:	mTriMesh(0), mMeshCount(0),
		mVertexCounts(0), mTriCounts(0), mVBs(0), mIBs(0),
		mVFormat( CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK ),
		mCeilY(0.0f),
		mColMesh( 0 )
	{
		int i;

		// 1 byte - has collision mesh?
		char hasColMesh;
		fread( &hasColMesh, 1, 1, f );
		// 4 bytes - mesh count
		fread( &mMeshCount, 1, 4, f );
		mVertexCounts = new int[ mMeshCount ];
		mTriCounts = new int[ mMeshCount ];
		mVBs = new CLevelMesh::TVertex*[ mMeshCount ];
		mIBs = new unsigned short*[ mMeshCount ];
		// [meshcount] - VB+IB
		for( i = 0; i < mMeshCount; ++i ) {
			fread( &mVertexCounts[i], 1, 4, f );	// 4b - vertex count
			fread( &mTriCounts[i], 1, 4, f );		// 4b - tri count
			// [vertex count] vertices
			mVBs[i] = new CLevelMesh::TVertex[ mVertexCounts[i] ];
			fread( mVBs[i], sizeof(CLevelMesh::TVertex), mVertexCounts[i], f );
			// [tricount*3] indices
			mIBs[i] = new unsigned short[ mTriCounts[i] * 3 ];
			fread( mIBs[i], 2*3, mTriCounts[i], f );
		}
		// if have col mesh - read it
		if( hasColMesh ) {
			int vcount, tricount;
			// 4 bytes - vertex count
			fread( &vcount, 1, 4, f );
			// 4 bytes - tri count
			fread( &tricount, 1, 4, f );
			// read VB
			SVector3* colVB = new SVector3[vcount];
			fread( colVB, sizeof(SVector3), vcount, f );
			// read IB
			int* colIB = new int[tricount*3];
			fread( colIB, 4*3, tricount, f );
			// create col mesh
			mColMesh = new CCollisionMesh( vcount, tricount, colVB, sizeof(SVector3), (const unsigned short*)colIB, true );
			delete[] colVB;
			delete[] colIB;
		}
	}
		
	CGameMapMeshData( const CTriangleMesh& triMesh, bool createColMesh, float ceilY )
	:	mTriMesh( &triMesh ), mMeshCount(0),
		mVertexCounts(0), mTriCounts(0), mVBs(0), mIBs(0),
		mVFormat( CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK ),
		mCeilY(ceilY),
		mColMesh( 0 )
	{
		calculate( createColMesh );
	}

	~CGameMapMeshData()
	{
		for( int i = 0; i < mMeshCount; ++i ) {
			safeDeleteArray( mVBs[i] );
			safeDeleteArray( mIBs[i] );
		}
		safeDeleteArray( mVertexCounts );
		safeDeleteArray( mTriCounts );
		safeDeleteArray( mVBs );
		safeDeleteArray( mIBs );
		safeDelete( mColMesh );
	}

	void	save( FILE* f ) const
	{
		int i;

		// 1 byte - has collision mesh?
		char hasColMesh = (mColMesh) ? 1 : 0;
		fwrite( &hasColMesh, 1, 1, f );
		// 4 bytes - mesh count
		fwrite( &mMeshCount, 1, 4, f );
		// [meshcount] - VB+IB
		for( i = 0; i < mMeshCount; ++i ) {
			fwrite( &mVertexCounts[i], 1, 4, f );	// 4b - vertex count
			fwrite( &mTriCounts[i], 1, 4, f );		// 4b - tri count
			// [vertex count] vertices
			fwrite( mVBs[i], sizeof(CLevelMesh::TVertex), mVertexCounts[i], f );
			// [tricount*3] indices
			fwrite( mIBs[i], 2*3, mTriCounts[i], f );
		}
		// if has col mesh - write it
		if( hasColMesh ) {
			int val;
			// 4 bytes - vertex count
			val = mColMesh->getVertexCount(); fwrite( &val, 1, 4, f );
			// 4 bytes - tri count
			val = mColMesh->getTriangleCount(); fwrite( &val, 1, 4, f );
			// [verts] - vertices
			fwrite( &mColMesh->getVertex(0), sizeof(SVector3), mColMesh->getVertexCount(), f );
			// [tris*3] - indices
			fwrite( mColMesh->getTriangle(0), 4*3, mColMesh->getTriangleCount(), f );
		}
	}

private:
	bool	isCeilVert( int i ) const {
		return mTriMesh->getVerts()[i].pos.y > mCeilY;
	}
	bool	isCeilFace( int i ) const {
		const CTriangleMesh::SFace& f = mTriMesh->getFaces()[i];
		return isCeilVert(f.verts[0]) || isCeilVert(f.verts[1]) || isCeilVert(f.verts[2]);
	}
	void calculate( bool createColMesh )
	{
		int i;

		assert( mMeshCount == 0 );

		int nsrcfaces = mTriMesh->getFaces().size();
		int nsrcverts = mTriMesh->getVerts().size();
		int ndstfaces = nsrcfaces;
		int ndstverts = nsrcverts;
		int* vertRemap = new int[ nsrcverts ];
		// loop through the mesh and calc real counts
		ndstfaces = 0;
		ndstverts = 0;
		int skipped = 0;
		for( i = 0; i < nsrcverts; ++i ) {
			vertRemap[i] = i - skipped;
			if( !isCeilVert(i) )
				++ndstverts;
			else
				++skipped;
		}
		for( i = 0; i < nsrcfaces; ++i ) {
			if( !isCeilFace(i) )
				++ndstfaces;
		}
		int ndstindices = ndstfaces * 3;

		CD3DVertexDecl* vdecl = RGET_VDECL(mVFormat);

		//
		// create one big D3DX mesh

		D3DVERTEXELEMENT9 vdcl[MAX_FVF_DECL_SIZE];
		UINT nelems;
		vdecl->getObject()->GetDeclaration( vdcl, &nelems );
		ID3DXMesh* m = 0;
		D3DXCreateMesh( ndstfaces, ndstverts, D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, vdcl, &CD3DDevice::getInstance().getDevice(), &m );


		// IB
		unsigned int* ib = 0;
		m->LockIndexBuffer( 0, (void**)&ib );
		for( i = 0; i < nsrcfaces; ++i ) {
			if( isCeilFace(i) )
				continue;
			const CTriangleMesh::SFace& f = mTriMesh->getFaces()[i];
			ib[0] = vertRemap[ f.verts[0] ];
			ib[1] = vertRemap[ f.verts[1] ];
			ib[2] = vertRemap[ f.verts[2] ];
			ib += 3;
		}
		m->UnlockIndexBuffer();
		// VB
		CLevelMesh::TVertex* vb = 0;
		m->LockVertexBuffer( 0, (void**)&vb );
		for( i = 0; i < nsrcverts; ++i ) {
			if( isCeilVert(i) )
				continue;
			const CTriangleMesh::SVertex& v = mTriMesh->getVerts()[i];
			vb->p = v.pos;
			// normal color encoded
			// w contains "perforable" factor
			SVector3 n = v.normal;
			n.x += 1.0f;	n.y += 1.0f;	n.z += 1.0f;
			n *= 127.5f;
			int nx = int(n.x) & 255;
			int ny = int(n.y) & 255;
			int nz = int(n.z) & 255;
			int nw = int(v.data.x * 255.0f) & 255;
			vb->diffuse = (nw<<24) | (nx<<16) | (ny<<8) | (nz<<0);
			++vb;
		}
		m->UnlockVertexBuffer();

		//
		// calculate adjacency

		DWORD* adj = new DWORD[ndstfaces*3];
		m->GenerateAdjacency( 0.0f, adj );
		DWORD* adjout = new DWORD[ndstfaces*3];
		m->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, adj, adjout, NULL, NULL );
		delete[] adj;
		adj = adjout;

		//
		// optionally create collision mesh

		if( createColMesh ) {
			m->LockIndexBuffer( D3DLOCK_READONLY, (void**)&ib );
			m->LockVertexBuffer( D3DLOCK_READONLY, (void**)&vb );
			mColMesh = new CCollisionMesh( ndstverts, ndstfaces, (const SVector3*)vb, m->GetNumBytesPerVertex(), (const unsigned short*)ib, true );
			m->UnlockIndexBuffer();
			m->UnlockVertexBuffer();
		}

		//
		// split into small meshes

		DWORD			meshesCount;
		ID3DXBuffer*	bufmeshes;
		ID3DXBuffer*	bufmeshesAdj;
		D3DXSplitMesh( m, adj, 5000, D3DXMESH_SYSTEMMEM, &meshesCount, &bufmeshes, &bufmeshesAdj, NULL, NULL );

		ID3DXMesh** meshes = static_cast<ID3DXMesh**>( bufmeshes->GetBufferPointer() );
		const DWORD** meshesAdj = static_cast<const DWORD**>( bufmeshesAdj->GetBufferPointer() );

		//
		// copy the small meshes data

		mMeshCount = meshesCount;
		mVertexCounts = new int[meshesCount];
		mTriCounts = new int[meshesCount];
		mVBs = new CLevelMesh::TVertex*[meshesCount];
		mIBs = new unsigned short*[meshesCount];
		for( i = 0; i < meshesCount; ++i ) {
			ID3DXMesh* mm = meshes[i];
			mVertexCounts[i] = mm->GetNumVertices();
			mTriCounts[i] = mm->GetNumFaces();

			//mm->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, meshesAdj[i], NULL, NULL, NULL );

			const unsigned short* mib = 0;
			mm->LockIndexBuffer( D3DLOCK_READONLY, (void**)&mib );
			mIBs[i] = new unsigned short[mTriCounts[i]*3];
			memcpy( mIBs[i], mib, mTriCounts[i]*3*2 );
			mm->UnlockIndexBuffer();

			const CLevelMesh::TVertex* mvb = 0;
			mm->LockVertexBuffer( D3DLOCK_READONLY, (void**)&mvb );
			mVBs[i] = new CLevelMesh::TVertex[mVertexCounts[i]];
			memcpy( mVBs[i], mvb, mVertexCounts[i]*sizeof(CLevelMesh::TVertex) );
			mm->UnlockVertexBuffer();
			
			mm->Release();
		}
		bufmeshes->Release();
		bufmeshesAdj->Release();

		delete[] adj;
		delete[] vertRemap;

		m->Release();
	}

public:
	const CTriangleMesh*	mTriMesh;
	int		mMeshCount;
	int*	mVertexCounts;
	int*	mTriCounts;
	CLevelMesh::TVertex**	mVBs;
	unsigned short**		mIBs;
	CVertexFormat			mVFormat;

	// -2.0f to turn off, -0.1 to discard ceiling, something like 0.6 to discard walls
	//float	mCeilNormalY; 
	// -BIG to turn off, 0.1 to discard upper part.
	float	mCeilY;

	CCollisionMesh*	mColMesh;
};


// --------------------------------------------------------------------------


class CGameMapMeshCreator : public IMeshCreator {
public:
	CGameMapMeshCreator( const CGameMapMeshData& data ) : mData( &data ) { }

	void setData( const CGameMapMeshData& data ) { mData = &data; }

	virtual void createMesh( CMesh& mesh ) {
		assert( !mesh.isCreated() );

		int i;
		int ngroups = mData->mMeshCount;
		int nverts = 0;
		int ntris = 0;
		for( i = 0; i < ngroups; ++i ) {
			nverts += mData->mVertexCounts[i];
			ntris += mData->mTriCounts[i];
		}

		mesh.createResource( nverts, ntris*3, mData->mVFormat, 2, *RGET_VDECL(mData->mVFormat), CMesh::BUF_STATIC );

		// now, assemble small meshes into this big one, and remember
		// groups
		unsigned short* pib = (unsigned short*)mesh.lockIBWrite();
		assert( pib );
		CLevelMesh::TVertex* pvb = (CLevelMesh::TVertex*)mesh.lockVBWrite();
		assert( pvb );

		int groupTri = 0;
		int groupVert = 0;
		for( i = 0; i < ngroups; ++i ) {
			memcpy( pib, mData->mIBs[i], mData->mTriCounts[i]*3*2 );
			pib += mData->mTriCounts[i]*3;
			memcpy( pvb, mData->mVBs[i], mData->mVertexCounts[i]*sizeof(CLevelMesh::TVertex) );
			pvb += mData->mVertexCounts[i];

			CMesh::CGroup group(groupVert,mData->mVertexCounts[i],groupTri,mData->mTriCounts[i]);
			mesh.addGroup( group );
			groupVert += mData->mVertexCounts[i];
			groupTri += mData->mTriCounts[i];
		}

		mesh.unlockIBWrite();
		mesh.unlockVBWrite();

		mesh.computeAABBs();
	}

private:
	const CGameMapMeshData*	mData;
};



// --------------------------------------------------------------------------


class CRenderableLevelMesh : public CRenderable {
public:
	CRenderableLevelMesh( CMesh& mesh, int priority = 0 );

	const CMesh& getMesh() const { return *mMesh; }

	virtual void render( const CRenderContext& ctx );
	
	virtual const CD3DVertexBuffer*	getUsedVB() const { return &mMesh->getVB(); }
	virtual const CD3DIndexBuffer*	getUsedIB() const { return &mMesh->getIB(); }
	
private:
	CMesh*	mMesh;
};


CRenderableLevelMesh::CRenderableLevelMesh( CMesh& mesh, int priority )
:	CRenderable( NULL, priority ),
	mMesh( &mesh )
{
	assert( mesh.isCreated() );
}

void CRenderableLevelMesh::render( const CRenderContext& ctx )
{
	assert( mMesh );

	HRESULT hr;

	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();

	device.setIndexBuffer( &mMesh->getIB() );
	device.setVertexBuffer( 0, &mMesh->getVB(), 0, mMesh->getVertexStride() );
	device.setDeclaration( mMesh->getVertexDecl() );

	const SMatrix4x4& viewProj = ctx.getCamera().getViewProjMatrix();

	int n = mMesh->getGroupCount();
	for( int i = 0; i < n; ++i ) {
		const CMesh::CGroup& group = mMesh->getGroup( i );

		// frustum cull
		if( group.getAABB().frustumCull( viewProj ) )
			continue;

		/*
		SVector4 col;
		col.x = fmodf( i*5.1f/n, 0.6f ) + 0.4f;
		col.y = fmodf( i*6.7f/n, 0.6f ) + 0.4f;
		col.z = fmodf( i*8.3f/n, 0.6f ) + 0.4f;
		col.w = 1.0f;
		getParams().getEffect()->getObject()->SetVector( "vColor", &col );
		*/
		
		hr = dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
			group.getFirstVertex(), 0, group.getVertexCount(),
			group.getFirstPrim() * 3, group.getPrimCount() );
		// stats
		stats.incVerticesRendered( group.getVertexCount() );
		stats.incPrimsRendered( group.getPrimCount() );
		stats.incDrawCalls();
		if( FAILED( hr ) ) {
			THROW_DXERROR( hr, "failed to DIP" );
		}
	}
}



// --------------------------------------------------------------------------

CLevelMesh::CLevelMesh( const CGameMap& gameMap )
:	mGameMap( &gameMap )
{
	mRenderableMinimap = new CRenderableMesh( *RGET_MESH("billboard"), 0, NULL, 0 );
	CEffectParams& rmep = mRenderableMinimap->getParams();
	rmep.setEffect( *RGET_FX("minimapLev") );
	rmep.addTexture( "tBase", *RGET_S_TEX(RID_TEX_LEVEL) );
	rmep.addVector4( "vSize", SVector4(
		gameMap.getCellsX(),
		gameMap.getCellsY(),
		gameMap.getCellsX()/2-0.5f,
		-gameMap.getCellsY()/2-0.5f)
	);

	int i;
	for( i = 0; i < GFX_DETAIL_LEVELS; ++i ) {
		mMeshTri[i] = NULL;
	}

	// try to load from cache
	mLoadedFromCache = loadCachedData();
	if( !mLoadedFromCache )
		computeData();

	const char* rids[MESHMODECOUNT] = { RID_MESH_LEVEL_FULL, RID_MESH_LEVEL_NOTOP };
	for( i = 0; i < MESHMODECOUNT; ++i ) {
		mMeshCreator[i] = new CGameMapMeshCreator( *mMeshData[gAppSettings.gfxDetail][i] );
		CSharedMeshBundle::getInstance().registerMesh( rids[i], *mMeshCreator[i] );
		CMesh* mesh = RGET_S_MESH(rids[i]);
		std::string desc = (i==NOTOP) ? "levelMega" : "level";
		for( int j = 0; j < RMCOUNT; ++j ) {
			mRenderable[i][j] = NULL;
			// may not have all rendermodes
			CModelDesc* mdesc = RTRYGET_MDESC( (RMODE_PREFIX[j] + desc) );
			if( !mdesc )
				continue;
			CRenderableLevelMesh* rr = new CRenderableLevelMesh( *mesh, mdesc->getRenderPriority(0) );
			mdesc->fillFxParams( 0, rr->getParams() );
			mRenderable[i][j] = rr;
		}
	}
}

CLevelMesh::~CLevelMesh()
{
	// cleanup
	int i;
	for( i = 0; i < GFX_DETAIL_LEVELS; ++i ) {
		for( int j = 0; j < MESHMODECOUNT; ++j ) {
			delete mMeshData[i][j];
		}
	}
	for( i = 0; i < MESHMODECOUNT; ++i ) {
		for( int j = 0; j < RMCOUNT; ++j )
			safeDelete( mRenderable[i][j] );
	}
	for( i = 0; i < GFX_DETAIL_LEVELS; ++i ) {
		safeDelete( mMeshTri[i] ); // can be null if loaded from cache
	}
	delete mRenderableMinimap;
}

void CLevelMesh::updateDetailLevel( int level )
{
	const char* rids[MESHMODECOUNT] = { RID_MESH_LEVEL_FULL, RID_MESH_LEVEL_NOTOP };
	for( int i = 0; i < MESHMODECOUNT; ++i ) {
		mMeshCreator[i]->setData( *mMeshData[level][i] );
		CMesh* m = RGET_S_MESH(rids[i]);
		m->deleteResource();
		mMeshCreator[i]->createMesh( *m );
	}
}


void CLevelMesh::computeData()
{
	int i;

	CONS << "Computing level mesh..." << endl;

	CSubdivMesh	meshSD;
	for( i = 0; i < GFX_DETAIL_LEVELS; ++i ) {
		mMeshTri[i] = new CTriangleMesh();
	}
	
	initSubdivMesh( meshSD );

	meshSD.smooth();

	meshSD.subdivide();

	meshSD.smooth();

	meshSD.computeNormals();

	mMeshTri[GFX_DETAIL_LEVELS-1]->initFromSubdivMesh( meshSD );

	const float OPTIMIZE_TOLS[GFX_DETAIL_LEVELS] = { 0.8f, 0.5f, 0.2f, 0.1f };
	for( i = GFX_DETAIL_LEVELS-1; i >= 0; --i ) {
		mMeshTri[i]->optimize( OPTIMIZE_TOLS[i] );
		if( i > 0 )
			*mMeshTri[i-1] = *mMeshTri[i];
	}

	float ceils[MESHMODECOUNT] = { 1000.0f, 0.1f };

	for( i = 0; i < GFX_DETAIL_LEVELS; ++i ) {
		for( int j = 0; j < MESHMODECOUNT; ++j ) {
			bool createColMesh = ( i == 1 && j == FULL );
			mMeshData[i][j] = new CGameMapMeshData( *mMeshTri[i], createColMesh, ceils[j] );
			if( createColMesh )
				mColMesh = mMeshData[i][j]->mColMesh;
		}
	}

	// save data to cache
	saveDataToCache();
}

const unsigned int CACHED_DATA_VERSION = 20050930;


bool CLevelMesh::loadCachedData()
{
	// do we cache?
	if( !gAppSettings.cacheLevels )
		return false;

	// open the file
	FILE* f = fopen( ("data/levelcache/" + mGameMap->getName() + ".cache").c_str(), "rb" );
	if( !f )
		return false;

	unsigned int val;
	// read and check cache version
	fread( &val, 1, 4, f );
	if( val != CACHED_DATA_VERSION ) {
		fclose( f );
		return false;
	}
	// read and check map's CRC
	fread( &val, 1, 4, f );
	if( val != mGameMap->getCRC() ) {
		fclose( f );
		return false;
	}
	// read all mesh datas
	CONS << "Reading level mesh from cache..." << endl;
	for( int lod = 0; lod < GFX_DETAIL_LEVELS; ++lod ) {
		for( int mod = 0; mod < MESHMODECOUNT; ++mod ) {
			mMeshData[lod][mod] = new CGameMapMeshData( f );
			if( mMeshData[lod][mod]->mColMesh )
				mColMesh = mMeshData[lod][mod]->mColMesh;
		}
	}

	return true;
}

void CLevelMesh::saveDataToCache()
{
	// do we cache?
	if( !gAppSettings.cacheLevels )
		return;

	// open the file
	FILE* f = fopen( ("data/levelcache/" + mGameMap->getName() + ".cache").c_str(), "wb" );
	if( !f )
		return;

	// write cache version
	fwrite( &CACHED_DATA_VERSION, 1, 4, f );

	// write CRC
	unsigned int mapCRC = mGameMap->getCRC();
	fwrite( &mapCRC, 1, 4, f );

	// write all mesh datas
	for( int lod = 0; lod < GFX_DETAIL_LEVELS; ++lod ) {
		for( int mod = 0; mod < MESHMODECOUNT; ++mod ) {
			CGameMapMeshData& data = *mMeshData[lod][mod];
			data.save( f );
		}
	}

	fclose( f );
	
	CONS << "Saved level mesh to cache." << endl;
}


void CLevelMesh::render( eRenderMode renderMode, eMeshMode meshMode )
{
	if( mRenderable[meshMode][renderMode] )
		G_RENDERCTX->attach( *mRenderable[meshMode][renderMode] );
}

void CLevelMesh::renderMinimap()
{
	G_RENDERCTX->attach( *mRenderableMinimap );
}

struct SCellVerts {
	int	ul, ur, dl, dr;
};


void CLevelMesh::initSubdivMesh( CSubdivMesh& mesh )
{
	const CGameMap& gmap = *mGameMap;
	CSubdivMesh::TVertexVector& mverts = mesh.getVerts();
	CSubdivMesh::TFaceVector& mfaces = mesh.getFaces();

	// create initial mesh

	int cellsx = gmap.getCellsX();
	int cellsz = gmap.getCellsY();

	SCellVerts* cellVerts = new SCellVerts[cellsx*cellsz];
	memset( cellVerts, -1, cellsx*cellsz*sizeof(SCellVerts) );

	for( int z = 1; z < cellsz-1; ++z ) {
		for( int x = 1; x < cellsx-1; ++x ) {
			const int cellidx = z * cellsx + x;

			const CGameMap::SCell& c = gmap.getCell( cellidx );

			//
			// just skip non-blood cells

			if( !gmap.isBlood(c.type) )
				continue;

			//
			// create verts for cell corners if needed

			const float y0 = -c.height;
			const float y1 = c.height;

			const int typeL = gmap.getCell(cellidx-1).type;
			const int typeR = gmap.getCell(cellidx+1).type;
			const int typeU = gmap.getCell(cellidx-cellsx).type;
			const int typeD = gmap.getCell(cellidx+cellsx).type;

			const int typeUL = gmap.getCell(cellidx-1-cellsx).type;
			const int typeUR = gmap.getCell(cellidx+1-cellsx).type;
			const int typeDL = gmap.getCell(cellidx-1+cellsx).type;
			const int typeDR = gmap.getCell(cellidx+1+cellsx).type;

			SCellVerts& vertsmine = cellVerts[cellidx];
			const SCellVerts& vertsl = cellVerts[cellidx-1];
			const SCellVerts& vertsu = cellVerts[cellidx-cellsx];
			const SCellVerts& vertsul = cellVerts[cellidx-cellsx-1];
			const SCellVerts& vertsur = cellVerts[cellidx-cellsx+1];
			vertsmine.ul = vertsu.dl;
			if( vertsmine.ul < 0 ) vertsmine.ul = vertsl.ur;
			if( vertsmine.ul < 0 ) vertsmine.ul = vertsul.dr;
			vertsmine.ur = vertsu.dr;
			if( vertsmine.ur < 0 ) vertsmine.ur = vertsur.dl;
			vertsmine.dl = vertsl.dr;
			if( vertsmine.ul < 0 ) {
				int vidx = mverts.size();
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts[vidx+0].pos.set( x-0.5f, y0, -(z-0.5f) );
				mverts[vidx+1].pos.set( x-0.5f, y1, -(z-0.5f) );
				SVector3 dt( c.otherType ? 1.0f : 0.0f, 0, 0 );
				mverts[vidx+0].data = dt;
				mverts[vidx+1].data = dt;
				vertsmine.ul = vidx;
			}
			if( vertsmine.ur < 0 ) {
				int vidx = mverts.size();
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts[vidx+0].pos.set( x+1-0.5f, y0, -(z-0.5f) );
				mverts[vidx+1].pos.set( x+1-0.5f, y1, -(z-0.5f) );
				SVector3 dt( c.otherType ? 1.0f : 0.0f, 0, 0 );
				mverts[vidx+0].data = dt;
				mverts[vidx+1].data = dt;
				vertsmine.ur = vidx;
			}
			if( vertsmine.dl < 0 ) {
				int vidx = mverts.size();
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts[vidx+0].pos.set( x-0.5f, y0, -(z+1-0.5f) );
				mverts[vidx+1].pos.set( x-0.5f, y1, -(z+1-0.5f) );
				SVector3 dt( c.otherType ? 1.0f : 0.0f, 0, 0 );
				mverts[vidx+0].data = dt;
				mverts[vidx+1].data = dt;
				vertsmine.dl = vidx;
			}
			{
				int vidx = mverts.size();
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts.push_back( CSubdivMesh::SVertex() );
				mverts[vidx+0].pos.set( x+1-0.5f, y0, -(z+1-0.5f) );
				mverts[vidx+1].pos.set( x+1-0.5f, y1, -(z+1-0.5f) );
				SVector3 dt( c.otherType ? 1.0f : 0.0f, 0, 0 );
				mverts[vidx+0].data = dt;
				mverts[vidx+1].data = dt;
				vertsmine.dr = vidx;
			}

			//
			// create faces

			CSubdivMesh::SFace f;
			// bottom
			f.verts[0] = vertsmine.ul;
			f.verts[1] = vertsmine.dl;
			f.verts[2] = vertsmine.dr;
			f.verts[3] = vertsmine.ur;
			mfaces.push_back( f );
			// top
			f.verts[0] = vertsmine.ul+1;
			f.verts[1] = vertsmine.ur+1;
			f.verts[2] = vertsmine.dr+1;
			f.verts[3] = vertsmine.dl+1;
			mfaces.push_back( f );
			// left (optional)
			if( !gmap.isBlood(typeL) ) {
				f.verts[0] = vertsmine.ul;
				f.verts[1] = vertsmine.ul+1;
				f.verts[2] = vertsmine.dl+1;
				f.verts[3] = vertsmine.dl;
				mfaces.push_back( f );
			}
			// right (optional)
			if( !gmap.isBlood(typeR) ) {
				f.verts[0] = vertsmine.ur+1;
				f.verts[1] = vertsmine.ur;
				f.verts[2] = vertsmine.dr;
				f.verts[3] = vertsmine.dr+1;
				mfaces.push_back( f );
			}
			// top (optional)
			if( !gmap.isBlood(typeU) ) {
				f.verts[0] = vertsmine.ul+1;
				f.verts[1] = vertsmine.ul;
				f.verts[2] = vertsmine.ur;
				f.verts[3] = vertsmine.ur+1;
				mfaces.push_back( f );
			}
			// bottom (optional)
			if( !gmap.isBlood(typeD) ) {
				f.verts[0] = vertsmine.dl;
				f.verts[1] = vertsmine.dl+1;
				f.verts[2] = vertsmine.dr+1;
				f.verts[3] = vertsmine.dr;
				mfaces.push_back( f );
			}
		}
	}

	delete[] cellVerts;
	
	mesh.fillHalfEdges();
	//mesh.checkValidity();

	mesh.computeNormals();
}



void CLevelMesh::fitSphere( SVector3& pos, float radius ) const
{
	mColSphere.setRadius( radius );
	int iters = 0;
	do {
		mColSphere.setPosition( pos );
		bool collided = mMeshSphereCollider.perform( *mColMesh, mColSphere, mColResult );
		if( !collided )
			return;
		pos += mColResult.mDirection * mColResult.mDistance * 0.5f;
	} while( ++iters < 5 );
}

void CLevelMesh::collideSphere( SVector3& pos, float radius, std::vector<int>& tris ) const
{
	mColSphere.setPosition( pos );
	mColSphere.setRadius( radius );
	mMeshSphereCollider.perform( *mColMesh, mColSphere, tris );
}

bool CLevelMesh::collideSphere( SVector3& pos, float radius ) const
{
	mColSphere.setPosition( pos );
	mColSphere.setRadius( radius );
	return mMeshSphereCollider.perform( *mColMesh, mColSphere, mColResult );
}
