#include "stdafx.h"
#include "WallPieces.h"
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/math/Plane.h>


void CWallPieces::debugRender( const SVector3* vb, CDebugRenderer& renderer, const bool* fractured )
{
	renderer.beginDebug();

	int npcs = getPieceCount();
	for( int i = 0; i < npcs; ++i ) {
		if( fractured[i] )
			continue;
		D3DCOLOR color;
		color = i * (71*65536+43*256+29);
		color &= 0x00ffffff;
		color |= 0xC0f0f0f0;
		const CWallPiece& pc = getPiece(i);
		renderer.renderTris( vb, sizeof(vb[0]), pc.getIB(), pc.getTriCount(), color );
	}

	renderer.endDebug();
}

void CWallPieces::debugRender( const SVector3* vb, CDebugRenderer& renderer, const std::vector<int>& pieces )
{
	renderer.beginDebug();

	int n = pieces.size();
	for( int i = 0; i < n; ++i ) {
		D3DCOLOR color;
		int idx = pieces[i];
		color = idx * (71*65536+43*256+29);
		color &= 0x00ffffff;
		color |= 0xff808080;
		const CWallPiece& pc = getPiece(idx);
		renderer.renderTris( vb, sizeof(vb[0]), pc.getIB(), pc.getTriCount(), color );
	}

	renderer.endDebug();
}


// --------------------------------------------------------------------------


CWall::CWall( const SVector2& size, float smallestElemSize )
: mPieces(size,smallestElemSize)
, mVB(NULL)
, mFracturedPieces(NULL)
, mLastFractureTime( -100.0f )
, mPiecesInited(false)
{
	mMatrix.identify();
}

CWall::~CWall()
{
	if( mVB ) delete[] mVB;
	safeDeleteArray( mFracturedPieces );
}

void CWall::initPieces()
{
	assert( !mPiecesInited );
	assert( !mFracturedPieces );
	int n = mPieces.getPieceCount();
	mFracturedPieces = new bool[n];
	for( int i = 0; i < n; ++i ) {
		mFracturedPieces[i] = false;
	}

	mPiecesInited = true;
}

void CWall::calcVB()
{
	if( mVB )
		return;

	int nvb = mPieces.getVerts().size();
	mVB = new SVector3[nvb];
	for( int i = 0; i < nvb; ++i ) {
		SVector3 p( mPieces.getVerts()[i].x, mPieces.getVerts()[i].y, 0 );
		D3DXVec3TransformCoord( &mVB[i], &p, &mMatrix );
	}
}


void CWall::debugRender( CDebugRenderer& renderer )
{
	if( !mVB )
		calcVB();
	mPieces.debugRender( mVB, renderer, mFracturedPieces );
}

void CWall::debugRender( CDebugRenderer& renderer, const std::vector<int>& pieces )
{
	if( !mVB )
		calcVB();
	mPieces.debugRender( mVB, renderer, pieces );
}


bool CWall::intersectRay( const SLine3& ray, float& t ) const
{
	SPlane plane;
	D3DXPlaneFromPointNormal( &plane, &mMatrix.getOrigin(), &mMatrix.getAxisZ() );
	SVector3 pt;
	if( !plane.intersect( ray, pt ) ) {
		return false;
	}
	t = ray.project( pt );
	if( t < 0 )
		return false;
	return true;
}


void CWall::fracturePiecesInSphere( float t, bool fractureOut, const SVector3& pos, float radius, std::vector<int>& pcs )
{
	if( !mPiecesInited )
		initPieces();

	if( fractureOut )
		mLastFractureTime = t;

	// to local space
	SVector3 locPos;
	D3DXVec3TransformCoord( &locPos, &pos, &mInvMatrix );

	// fetch the pieces
	pcs.resize( 0 );

	// TODO: optimize, right now linear search!
	int n = mPieces.getPieceCount();
	for( int i = 0; i < n; ++i ) {
		const CWallPiece& p = mPieces.getPiece( i );
		if( mFracturedPieces[i] )
			continue;
		SVector2 c = p.getAABB().getCenter();
		SVector3 tocenter = locPos - SVector3(c.x,c.y,0);
		if( tocenter.lengthSq() < radius*radius ) {
			pcs.push_back( i );
			if( fractureOut )
				mFracturedPieces[i] = true;
		}
	}
}


void CWall::update( float t )
{
	if( !mPiecesInited )
		initPieces();

	const float RESTORE_TIME = 5.0f;
	if( t > mLastFractureTime + RESTORE_TIME ) {
		// TODO: optimize!
		int n = mPieces.getPieceCount();
		for( int i = 0; i < n; ++i ) {
			mFracturedPieces[i] = false;
		}
	}
}
