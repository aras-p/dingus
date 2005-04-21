#include "stdafx.h"
#include "WallPieces.h"
#include <dingus/gfx/DebugRenderer.h>
#include <dingus/math/Plane.h>


void CWall2D::debugRender( const SVector3* vb, CDebugRenderer& renderer, const bool* fractured )
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
		const CWallPiece2D& pc = getPiece(i);
		renderer.renderTris( vb, sizeof(vb[0]), pc.getIB(), pc.getTriCount(), color );
	}

	renderer.endDebug();
}

void CWall2D::debugRender( const SVector3* vb, CDebugRenderer& renderer, const std::vector<int>& pieces )
{
	renderer.beginDebug();

	int n = pieces.size();
	for( int i = 0; i < n; ++i ) {
		D3DCOLOR color;
		int idx = pieces[i];
		color = idx * (71*65536+43*256+29);
		color &= 0x00ffffff;
		color |= 0xff808080;
		const CWallPiece2D& pc = getPiece(idx);
		renderer.renderTris( vb, sizeof(vb[0]), pc.getIB(), pc.getTriCount(), color );
	}

	renderer.endDebug();
}



// --------------------------------------------------------------------------


void CWallPiece3D::init( const CWall3D& w, int idx )
{
	const CWallPiece2D& piece = w.getWall2D().getPiece(idx);

	const float HALF_THICK = 0.02f;
	
	// construct VB/IB for this piece, with positions centered
	SVector2 pcenter = piece.getAABB().getCenter();

	static std::vector<int>	vertRemap;
	vertRemap.resize(0);
	vertRemap.resize( w.getWall2D().getVerts().size(), -1 );

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

			SVector2 pos = w.getWall2D().getVerts()[oldIdx];
			pos -= pcenter;
			SVertexXyzNormal vtx;
			vtx.p.set( pos.x, pos.y, -HALF_THICK );
			vtx.n.set( 0, 0, -1 );
			mVB.push_back( vtx );
		}
		mIB.push_back( newIdx );
	}
	// construct another side
	int nverts = mVB.size();
	for( i = 0; i < nverts; ++i ) {
		SVertexXyzNormal vtx = mVB[i];
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
		SVertexXyzNormal v0 = mVB[idx0];
		SVertexXyzNormal v1 = mVB[idx1];
		SVertexXyzNormal v2 = mVB[idx0+nverts];
		SVertexXyzNormal v3 = mVB[idx1+nverts];
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


CWall3D::CWall3D( const SVector2& size, float smallestElemSize )
: mWall2D(size,smallestElemSize)
, mVB(NULL)
, mPieces3D(NULL)
, mFracturedPieces(NULL)
, mLastFractureTime( -100.0f )
, mPiecesInited(false)
{
	mMatrix.identify();
}

CWall3D::~CWall3D()
{
	if( mVB ) delete[] mVB;
	safeDeleteArray( mPieces3D );
	safeDeleteArray( mFracturedPieces );
}

void CWall3D::initPieces()
{
	assert( !mPiecesInited );
	assert( !mFracturedPieces );
	assert( !mPieces3D );
	
	int n = mWall2D.getPieceCount();
	
	mPieces3D = new CWallPiece3D[n];
	mFracturedPieces = new bool[n];

	for( int i = 0; i < n; ++i ) {
		mPieces3D[i].init( *this, i );
		mFracturedPieces[i] = false;
	}

	mPiecesInited = true;
}

void CWall3D::calcVB()
{
	if( mVB )
		return;

	int nvb = mWall2D.getVerts().size();
	mVB = new SVector3[nvb];
	for( int i = 0; i < nvb; ++i ) {
		SVector3 p( mWall2D.getVerts()[i].x, mWall2D.getVerts()[i].y, 0 );
		D3DXVec3TransformCoord( &mVB[i], &p, &mMatrix );
	}
}


void CWall3D::debugRender( CDebugRenderer& renderer )
{
	if( !mVB )
		calcVB();
	mWall2D.debugRender( mVB, renderer, mFracturedPieces );
}

void CWall3D::debugRender( CDebugRenderer& renderer, const std::vector<int>& pieces )
{
	if( !mVB )
		calcVB();
	mWall2D.debugRender( mVB, renderer, pieces );
}


bool CWall3D::intersectRay( const SLine3& ray, float& t ) const
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


void CWall3D::fracturePiecesInSphere( float t, bool fractureOut, const SVector3& pos, float radius, std::vector<int>& pcs )
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
	int n = mWall2D.getPieceCount();
	for( int i = 0; i < n; ++i ) {
		const CWallPiece2D& p = mWall2D.getPiece( i );
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


void CWall3D::update( float t )
{
	if( !mPiecesInited )
		initPieces();

	const float RESTORE_TIME = 5.0f;
	if( t > mLastFractureTime + RESTORE_TIME ) {
		// TODO: optimize!
		int n = mWall2D.getPieceCount();
		for( int i = 0; i < n; ++i ) {
			mFracturedPieces[i] = false;
		}
	}
}
