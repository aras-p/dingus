#include "stdafx.h"
#include "WallPhysics.h"
#include "WallPieces.h"
#include "Physics.h"
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>
#include <dingus/utils/CpuTimer.h>
#include <dingus/utils/Random.h>
#include <dingus/renderer/RenderableBuffer.h>


namespace {

	const float	GRAVITY = -5.0f;

	float	updateDT;


	typedef SVertexXyzNormalDiffuse TPieceVertex;
	const DWORD WALL_VERTEX_FVF = FVF_XYZ_NORMAL_DIFFUSE;


	// Resting piece in the walls
	class CRestingPiece : public boost::noncopyable {
	public:
		typedef std::vector<TPieceVertex>	TVertexVector;
		typedef std::vector<int>			TIntVector;

	public:
		CRestingPiece( const CWall& w, int idx );

		const TVertexVector& getVB() const { return mVB; }
		const TIntVector& getIB() const { return mIB; }

		const SMatrix4x4& getMatrix() const { return mMatrix; }
		const SVector3& getSize() const { return mSize; }

	private:
		SMatrix4x4		mMatrix; // Initial matrix
		TVertexVector	mVB;
		TIntVector		mIB;
		SVector3		mSize;
	};



	struct SWall : public boost::noncopyable {
	public:
		SWall() { }
		~SWall() {
			stl_utils::wipe( pieces );
		}
	public:
		CWall*	wall;
		std::vector<CRestingPiece*>	pieces;
	};
	typedef std::vector<SWall*>	TWallVector;
	TWallVector	walls;



	// Piece in physics
	class CPhysPiece : public physics::CPhysObject {
	public:
		CPhysPiece( const CRestingPiece& rpiece );
		~CPhysPiece();

		/// @return false if already dead
		bool	update();

		void	preRender( int& vbcount, int& ibcount ) const;
		void	render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const;

	private:
		SMatrix4x4				mMatrix;
		const CRestingPiece*	mRestPiece;
		float					mTimeLeft;
	};

	
	typedef fastvector<CPhysPiece*>	TPieceVector;
	TPieceVector	pieces;


	CRenderableIndexedBuffer*	renderable;
	TVBChunk::TSharedPtr		vbChunk;
	TIBChunk::TSharedPtr		ibChunk;
	bool	renderIntoVB();


	wall_phys::SStats	stats;

};



// --------------------------------------------------------------------------


CRestingPiece::CRestingPiece( const CWall& w, int idx )
{
	const CWallPiece& piece = w.getPieces().getPiece(idx);

	const float HALF_THICK = 0.02f;
	
	// construct VB/IB for this piece, with positions centered
	SVector2 pcenter = piece.getAABB().getCenter();

	static std::vector<int>	vertRemap;
	vertRemap.resize(0);
	vertRemap.resize( w.getPieces().getVerts().size(), -1 );

	int i;

	int nidx = piece.getTriCount()*3;
	mIB.reserve( nidx * 2 + piece.getVertexCount()*6 );
	mVB.reserve( piece.getVertexCount()*6 );

	// construct one side
	for( i = 0; i < nidx; ++i ) {
		int oldIdx = piece.getIB()[i];
		int newIdx = vertRemap[oldIdx];
		if( newIdx < 0 ) {
			newIdx = mVB.size();
			vertRemap[oldIdx] = newIdx;

			SVector2 pos = w.getPieces().getVerts()[oldIdx];
			pos -= pcenter;
			TPieceVertex vtx;
			vtx.p.set( pos.x, pos.y, -HALF_THICK );
			vtx.n.set( 0, 0, -1 );
			mVB.push_back( vtx );
		}
		mIB.push_back( newIdx );
	}
	// construct another side
	int nverts = mVB.size();
	for( i = 0; i < nverts; ++i ) {
		TPieceVertex vtx = mVB[i];
		vtx.p.z = -vtx.p.z;
		vtx.n.z = -vtx.n.z;
		mVB.push_back( vtx );
	}
	for( i = 0; i < nidx/3; ++i ) {
		int idx0 = mIB[i*3+0];
		int idx1 = mIB[i*3+1];
		int idx2 = mIB[i*3+2];
		mIB.push_back( idx0 + nverts );
		mIB.push_back( idx2 + nverts );
		mIB.push_back( idx1 + nverts );
	}
	// construct side caps
	assert( nverts == piece.getVertexCount() );
	for( i = 0; i < nverts; ++i ) {
		int oldIdx0 = piece.getPolygon()[i];
		int oldIdx1 = piece.getPolygon()[(i+1)%nverts];
		int idx0 = vertRemap[oldIdx0];
		int idx1 = vertRemap[oldIdx1];
		assert( idx0 >= 0 && idx0 < nverts );
		assert( idx1 >= 0 && idx1 < nverts );
		TPieceVertex v0 = mVB[idx0];
		TPieceVertex v1 = mVB[idx1];
		TPieceVertex v2 = mVB[idx0+nverts];
		TPieceVertex v3 = mVB[idx1+nverts];
		SVector3 edge01 = v1.p - v0.p;
		SVector3 edge02 = v2.p - v0.p;
		SVector3 normal = edge01.cross( edge02 ).getNormalized();
		v0.n = v1.n = v2.n = v3.n = normal;
		mVB.push_back( v0 );
		mVB.push_back( v1 );
		mVB.push_back( v2 );
		mVB.push_back( v3 );
		mIB.push_back( nverts*2 + i*4 + 0 );
		mIB.push_back( nverts*2 + i*4 + 2 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 2 );
		mIB.push_back( nverts*2 + i*4 + 3 );
	}

	// construct initial mMatrix
	mMatrix.identify();
	mMatrix = w.getMatrix();
	mMatrix.getOrigin() += mMatrix.getAxisX() * pcenter.x;
	mMatrix.getOrigin() += mMatrix.getAxisY() * pcenter.y;

	mSize.set( piece.getAABB().getSize().x, piece.getAABB().getSize().y, HALF_THICK*2 );
}



// --------------------------------------------------------------------------


CPhysPiece::CPhysPiece( const CRestingPiece& rpiece )
:	physics::CPhysObject( rpiece.getMatrix(), rpiece.getSize() )
,	mRestPiece( &rpiece )
,	mTimeLeft( gRandom.getFloat(5.0f,25.0f) )
{
	mMatrix = rpiece.getMatrix();
}

CPhysPiece::~CPhysPiece()
{
}

bool CPhysPiece::update()
{
	mTimeLeft -= updateDT;
	if( mTimeLeft < 0 )
		return false;

	CPhysObject::update( mMatrix );
	return true;
}


void CPhysPiece::preRender( int& vbcount, int& ibcount ) const
{
	vbcount = mRestPiece->getVB().size();
	ibcount = mRestPiece->getIB().size();
}


void CPhysPiece::render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
{
	int i;
	
	// VB
	const TPieceVertex* srcVB = &mRestPiece->getVB()[0];
	vbcount = mRestPiece->getVB().size();
	for( i = 0; i < vbcount; ++i ) {
		SVector3 p, n;
		D3DXVec3TransformCoord( &p, &srcVB->p, &mMatrix );
		D3DXVec3TransformNormal( &n, &srcVB->n, &mMatrix );
		vb->p = p;
		vb->n = n;
		vb->diffuse = 0xFFff8000; // TBD
		++srcVB;
		++vb;
	}

	// IB
	const int* srcIB = &mRestPiece->getIB()[0];
	ibcount = mRestPiece->getIB().size();
	for( i = 0; i < ibcount; ++i ) {
		int idx = *srcIB + baseIndex;
		assert( idx >= 0 && idx < 64000 );
		*ib = idx;
		++srcIB;
		++ib;
	}
}



// --------------------------------------------------------------------------


void wall_phys::initialize( float updDT, const SVector3& boundMin, const SVector3& boundMax )
{
	renderable = new CRenderableIndexedBuffer( NULL, 0 );
	renderable->getParams().setEffect( *RGET_FX("wallPieces") );
	
	updateDT = updDT;

	walls.reserve( 6 );

	physics::initialize( updDT, GRAVITY, boundMin, boundMax );
}


int wall_phys::addWall( CWall& wall )
{
	SWall* w = new SWall();
	w->wall = &wall;

	int npcs = wall.getPieces().getPieceCount();
	for( int i = 0; i < npcs; ++i ) {
		CRestingPiece* rpc = new CRestingPiece( wall, i );
		w->pieces.push_back( rpc );
	}

	physics::addPlane( wall.getMatrix() );

	walls.push_back( w );
	return walls.size()-1;
}


void wall_phys::shutdown()
{
	stl_utils::wipe( pieces );
	stl_utils::wipe( walls );

	physics::shutdown();

	safeDelete( renderable );
}


void wall_phys::spawnPiece( int wallID, int index )
{
	assert( wallID >= 0 && wallID < walls.size() );
	CPhysPiece* p = new CPhysPiece( *walls[wallID]->pieces[index] );
	pieces.push_back( p );
}


void wall_phys::update()
{
	stats.pieceCount = pieces.size();

	physics::update1();

	{
		cputimer::ticks_type t1 = cputimer::ticks();
		// update pieces
		TPieceVector::iterator it, itEnd = pieces.end();
		for( it = pieces.begin(); it != itEnd;  ) {
			CPhysPiece* p = *it;
			if( !p->update() ) {
				it = pieces.erase( it );
				itEnd = pieces.end();
				delete p;
			} else {
				++it;
			}
		}
		cputimer::ticks_type t2 = cputimer::ticks();
		stats.msUpdate = double(t2-t1) * cputimer::secsPerTick() * 1000.0f;
	}

	physics::update2();

	stats.msColl = physics::getStats().msColl;
	stats.msPhys = physics::getStats().msPhys;
	
	renderIntoVB();
}

namespace {
bool renderIntoVB()
{
	int nverts = 0, nindices = 0;
	vbChunk = NULL;
	ibChunk = NULL;

	int i;

	// accumulate total vertex/index count
	int n = pieces.size();
	for( i = 0; i < n; ++i ) {
		int nvb, nib;
		pieces[i]->preRender( nvb, nib );
		nverts += nvb;
		nindices += nib;
	}

	if( !nverts || !nindices )
		return false;

	// lock and render
	vbChunk = CDynamicVBManager::getInstance().allocateChunk( nverts, sizeof(TPieceVertex) );
	ibChunk = CDynamicIBManager::getInstance().allocateChunk( nindices, 2 );
	TPieceVertex* vb = (TPieceVertex*)vbChunk->getData();
	unsigned short* ib = (unsigned short*)ibChunk->getData();
	int baseIndex = 0;
	for( i = 0; i < n; ++i ) {
		int nvb, nib;
		pieces[i]->render( vb, ib, baseIndex, nvb, nib );
		baseIndex += nvb;
		vb += nvb;
		ib += nib;
	}
	vbChunk->unlock();
	ibChunk->unlock();

	// setup renderable
	renderable->resetVBs();

	renderable->setVB( vbChunk->getBuffer(), 0 );
	renderable->setStride( vbChunk->getStride(), 0 );
	renderable->setBaseVertex( vbChunk->getOffset() );
	renderable->setMinVertex( 0 );
	renderable->setNumVertices( vbChunk->getSize() );

	renderable->setIB( ibChunk->getBuffer() );
	renderable->setStartIndex( ibChunk->getOffset() );
	renderable->setPrimCount( ibChunk->getSize() / 3 );

	renderable->setPrimType( D3DPT_TRIANGLELIST );
	return true;
}
}

void wall_phys::render( eRenderMode rm )
{
	if( pieces.empty() )
		return;

	if( rm != RM_NORMAL /*&& rm != RM_REFLECTED*/ )
		return;

	if( !vbChunk || !vbChunk->isValid() ||
		!ibChunk || !ibChunk->isValid() )
	{
		renderIntoVB();
	}
	G_RENDERCTX->attach( *renderable );
}

const wall_phys::SStats& wall_phys::getStats()
{
	return stats;
}
