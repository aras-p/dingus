#include "stdafx.h"
#include "WallPhysics.h"
#include "WallPieces.h"
#include "Physics.h"
#include <dingus/math/MathUtils.h>
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>
//#include <dingus/utils/CpuTimer.h>
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
		CPhysPiece( const CWallPiece3D& rpiece, bool longLived );
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

	int				lastActivePieceCount = 0;

	CRenderableIndexedBuffer*	renderables[RMCOUNT];

	TVBChunk::TSharedPtr		vbChunk;
	TIBChunk::TSharedPtr		ibChunk;
	bool	needsRendering = false;
	bool	renderIntoVB();


	wall_phys::SStats	stats;

};



// --------------------------------------------------------------------------


CPhysPiece::CPhysPiece( const CWallPiece3D& rpiece, bool longLived )
:	physics::CPhysObject( rpiece.getMatrix(), rpiece.getSize() )
,	mRestPiece( &rpiece )
,	mTimeLeft( longLived ? 60.0f : gRandom.getFloat(5.0f,25.0f) )
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
	const float FADE_TIME = 1.0f;
	mRestPiece->preRender( vbcount, ibcount );
}


void CPhysPiece::render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
{
	const float FADE_TIME = 2.0f;
	BYTE alpha = 255;
	if( mTimeLeft < FADE_TIME ) {
		float a = mTimeLeft/FADE_TIME;
		alpha = int(a*255.0f);
	}
	mRestPiece->render( mMatrix, vb, ib, baseIndex, vbcount, ibcount, alpha );
}



// --------------------------------------------------------------------------


void wall_phys::initialize( float updDT, const SVector3& boundMin, const SVector3& boundMax )
{
	for( int i = 0; i < RMCOUNT; ++i )
		renderables[i] = NULL;

	renderables[RM_NORMAL] = new CRenderableIndexedBuffer( NULL, 1 );
	renderables[RM_NORMAL]->getParams().setEffect( *RGET_FX("wall_DnSA") );
	renderables[RM_NORMAL]->getParams().addTexture( "tShadow", *RGET_S_TEX(RT_SHADOWBLUR) );
	renderables[RM_NORMAL]->getParams().addVector3( "vLightPos", LIGHT_POS_1 + LIGHT_WOFF );
	renderables[RM_CASTERSIMPLE] = new CRenderableIndexedBuffer( NULL, 1 );
	renderables[RM_CASTERSIMPLE]->getParams().setEffect( *RGET_FX("casterSimpleA") );
	
	updateDT = updDT;

	walls[0].reserve( 6 );
	walls[1].reserve( 6 );

	lastActivePieceCount = 0;

	physics::initialize( updDT, GRAVITY, boundMin, boundMax );
}


void wall_phys::addWall( int lodIndex, const CWall3D& wall )
{
	if( lodIndex == 0 )
		physics::addPlane( wall.getMatrix() );

	assert( lodIndex >= 0 && lodIndex < MAX_LODS );
	walls[lodIndex].push_back( &wall );
}

void wall_phys::addStaticWall( int lodIndex, const SMatrix4x4& matrix )
{
	if( lodIndex == 0 )
		physics::addPlane( matrix );

	assert( lodIndex >= 0 && lodIndex < MAX_LODS );
	walls[lodIndex].push_back( NULL );
}


void wall_phys::shutdown()
{
	stl_utils::wipe( pieces );
	walls[0].clear();
	walls[1].clear();

	physics::shutdown();

	for( int i = 0; i < RMCOUNT; ++i )
		safeDelete( renderables[i] );
}


void wall_phys::spawnPiece( int lodIndex, int wallID, int index, bool longLived )
{
	// if we have lots of pieces by now, spawn less!
	if( lastActivePieceCount > 400 ) {
		if( gRandom.getUInt()&3 )
			return;
	} else if( lastActivePieceCount > 200 ) {
		if( gRandom.getUInt()&1 )
			return;
	}
	
	assert( lodIndex >= 0 && lodIndex < MAX_LODS );
	assert( wallID >= 0 && wallID < walls[lodIndex].size() );
	assert( walls[lodIndex][wallID] );
	CPhysPiece* p = new CPhysPiece( walls[lodIndex][wallID]->getPieces3D()[index], longLived );
	pieces.push_back( p );
}

void wall_phys::clearPieces()
{
	stl_utils::wipe( pieces );
}


void wall_phys::update()
{
	stats.pieceCount = pieces.size();

	int physicsLod = 0;
	if( lastActivePieceCount > 900 )
		physicsLod = 4;
	else if( lastActivePieceCount > 700 )
		physicsLod = 3;
	else if( lastActivePieceCount > 600 )
		physicsLod = 2;
	else if( lastActivePieceCount > 400 )
		physicsLod = 1;

	physics::update( physicsLod );


	{
		//cputimer::ticks_type t1 = cputimer::ticks();
		// update pieces
		lastActivePieceCount = 0;
		TPieceVector::iterator it, itEnd = pieces.end();
		for( it = pieces.begin(); it != itEnd;  ) {
			CPhysPiece* p = *it;
			if( !p->update() ) {
				it = pieces.erase( it );
				itEnd = pieces.end();
				delete p;
			} else {
				if( !p->isRemoved() )
					++lastActivePieceCount;
				++it;
			}
		}
		//cputimer::ticks_type t2 = cputimer::ticks();
		//stats.msUpdate = double(t2-t1) * cputimer::secsPerTick() * 1000.0f;
	}

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

	// setup renderables
	CD3DVertexDecl* vdecl = RGET_VDECL( CVertexFormat( CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK ) );
	for( i = 0; i < RMCOUNT; ++i ) {
		if( !renderables[i] )
			continue;

		renderables[i]->resetVBs();

		renderables[i]->setVB( vbChunk->getBuffer(), 0 );
		renderables[i]->setStride( vbChunk->getStride(), 0 );
		renderables[i]->setBaseVertex( vbChunk->getOffset() );
		renderables[i]->setMinVertex( 0 );
		renderables[i]->setNumVertices( vbChunk->getSize() );

		renderables[i]->setIB( ibChunk->getBuffer() );
		renderables[i]->setStartIndex( ibChunk->getOffset() );
		renderables[i]->setPrimCount( ibChunk->getSize() / 3 );

		renderables[i]->setPrimType( D3DPT_TRIANGLELIST );
		renderables[i]->setVertexDecl( vdecl );
	}
	return true;
}
}

void wall_phys::render( eRenderMode rm )
{
	if( pieces.empty() )
		return;

	if( !renderables[rm] )
		return;

	if( needsRendering ||
		!vbChunk || !vbChunk->isValid() ||
		!ibChunk || !ibChunk->isValid() )
	{
		renderIntoVB();
	}

	G_RENDERCTX->attach( *renderables[rm] );
}


const wall_phys::SStats& wall_phys::getStats()
{
	return stats;
}
