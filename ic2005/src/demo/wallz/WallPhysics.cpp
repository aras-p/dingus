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



	const int MAX_LODS = 2;

	std::vector<const CWall3D*>	walls[MAX_LODS];



	// Piece in physics
	class CPhysPiece : public physics::CPhysObject {
	public:
		CPhysPiece( const CWallPiece3D& rpiece );
		~CPhysPiece();

		/// @return false if already dead
		bool	update();

		void	preRender( int& vbcount, int& ibcount ) const;
		void	render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const;

	private:
		SMatrix4x4				mMatrix;
		const CWallPiece3D*	mRestPiece;
		float					mTimeLeft;
	};

	
	typedef fastvector<CPhysPiece*>	TPieceVector;
	TPieceVector	pieces;


	CRenderableIndexedBuffer*	renderable;
	TVBChunk::TSharedPtr		vbChunk;
	TIBChunk::TSharedPtr		ibChunk;
	bool	needsRendering = false;
	bool	renderIntoVB();


	wall_phys::SStats	stats;

};



// --------------------------------------------------------------------------


CPhysPiece::CPhysPiece( const CWallPiece3D& rpiece )
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

	if( isIdle() )
		removeFromPhysics();
	CPhysObject::update( mMatrix );
	return true;
}


void CPhysPiece::preRender( int& vbcount, int& ibcount ) const
{
	mRestPiece->preRender( vbcount, ibcount );
}


void CPhysPiece::render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
{
	mRestPiece->render( mMatrix, vb, ib, baseIndex, vbcount, ibcount );
}



// --------------------------------------------------------------------------


void wall_phys::initialize( float updDT, const SVector3& boundMin, const SVector3& boundMax )
{
	renderable = new CRenderableIndexedBuffer( NULL, 0 );
	renderable->getParams().setEffect( *RGET_FX("wallPieces") );
	
	updateDT = updDT;

	walls[0].reserve( 6 );
	walls[1].reserve( 6 );

	physics::initialize( updDT, GRAVITY, boundMin, boundMax );
}


void wall_phys::addWall( int lodIndex, const CWall3D& wall )
{
	if( lodIndex == 0 )
		physics::addPlane( wall.getMatrix() );

	assert( lodIndex >= 0 && lodIndex < MAX_LODS );
	walls[lodIndex].push_back( &wall );
}


void wall_phys::shutdown()
{
	stl_utils::wipe( pieces );
	walls[0].clear();
	walls[1].clear();

	physics::shutdown();

	safeDelete( renderable );
}


void wall_phys::spawnPiece( int lodIndex, int wallID, int index )
{
	assert( lodIndex >= 0 && lodIndex < MAX_LODS );
	assert( wallID >= 0 && wallID < walls[lodIndex].size() );
	CPhysPiece* p = new CPhysPiece( walls[lodIndex][wallID]->getPieces3D()[index] );
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
	
	needsRendering = true;
}

namespace {
bool renderIntoVB()
{
	needsRendering = false;

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

	stats.vertexCount = nverts;
	stats.triCount = nindices/3;

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

	if( needsRendering ||
		!vbChunk || !vbChunk->isValid() ||
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
