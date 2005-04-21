#ifndef __WALL_PIECES_H
#define __WALL_PIECES_H

#include <dingus/math/Vector2.h>
#include <dingus/math/Line3.h>
#include "AABox2.h"
#include "Triangulate.h"

typedef std::vector<SVector2>	TWallVertexVector;

namespace dingus {
	class CDebugRenderer;
};


// --------------------------------------------------------------------------

/// Single piece of precomputed fracture, sitting in the wall.
class CWallPiece {
public:
	CWallPiece()
	{
	}
	CWallPiece( const CWallPiece& r )
		: mPolygon(r.mPolygon)
		, mTris(r.mTris)
		, mAABB(r.mAABB)
	{
	}
	void operator=( const CWallPiece& r )
	{
		if( this == &r )
			return;
		mPolygon = r.mPolygon;
		mTris = r.mTris;
		mAABB = r.mAABB;
	}

	void	addVertex( int v ) {
		mPolygon.push_back( v );
	}

	void	finish( const TWallVertexVector& verts ) {
		// triangulate polygon
		assert( mTris.empty() );
		std::vector<int>	polygonIB;
		bool trisOk = triangulator::process( verts, mPolygon, mTris );
		assert( trisOk );

		// calculate AABB
		int n = mPolygon.size();
		for( int i = 0; i < n; ++i ) {
			mAABB.extend( verts[mPolygon[i]] );
		}
	}

	int getTriCount() const { return mTris.size()/3; }
	const int* getIB() const { return &mTris[0]; }
	int getVertexCount() const { return mPolygon.size(); }
	const int* getPolygon() const { return &mPolygon[0]; }

	const CAABox2& getAABB() const { return mAABB; }

private:
	std::vector<int>	mPolygon;	///< Polygon vertex indices of the piece
	std::vector<int>	mTris;		///< Triangulated piece, 3 indices/tri
	CAABox2				mAABB;		///< 2D AABB of the piece
};


// --------------------------------------------------------------------------


/// Single wall with precomputed fracture pieces in 2D.
class CWallPieces : public boost::noncopyable {
public:
	CWallPieces( const SVector2& size, float smallestElemSize )
		: mSize( size )
		, mSmallestElemSize( smallestElemSize )
	{
	}

	void	addVertex( const SVector2& v ) {
		mVerts.push_back( v );
	}
	const TWallVertexVector& getVerts() const { return mVerts; }

	void	addPiece( const CWallPiece& piece ) {
		mPieces.push_back( piece );
	}
	int getPieceCount() const { return mPieces.size(); }
	const CWallPiece& getPiece( int i ) const { return mPieces[i]; }

	const SVector2& getSize() const { return mSize; }
	float getSmallestElemSize() const { return mSmallestElemSize; }

	void	debugRender( const SVector3* vb, CDebugRenderer& renderer, const bool* fractured );
	void	debugRender( const SVector3* vb, CDebugRenderer& renderer, const std::vector<int>& pieces );
	
private:
	TWallVertexVector		mVerts;
	std::vector<CWallPiece>	mPieces;
	SVector2		mSize;				///< Whole wall spans 0 to mSize range
	float			mSmallestElemSize;
};


// --------------------------------------------------------------------------


/**
 *	Single wall with precomputed fracture pieces, positioned in 3D and
 *  tracking fractured out pieces' state.
 */ 
class CWall : public boost::noncopyable {
public:
	CWall( const SVector2& size, float smallestElemSize );
	~CWall();

	const CWallPieces& getPieces() const { return mPieces; }
	CWallPieces& getPieces() { return mPieces; }

	const SMatrix4x4& getMatrix() const { return mMatrix; }
	void setMatrix( const SMatrix4x4& m ) { mMatrix = m; mInvMatrix = m; mInvMatrix.invert(); }

	void	calcVB();

	void	update( float t );

	bool	intersectRay( const SLine3& ray, float& t ) const;
	void	fracturePiecesInSphere( float t, bool fractureOut, const SVector3& pos, float radius, std::vector<int>& pcs );

	void	debugRender( CDebugRenderer& renderer );
	void	debugRender( CDebugRenderer& renderer, const std::vector<int>& pieces );

private:
	void	initPieces();

private:
	CWallPieces	mPieces;
	SMatrix4x4	mMatrix;
	SMatrix4x4	mInvMatrix;
	SVector3*	mVB;

	bool*		mFracturedPieces;
	float		mLastFractureTime;

	bool		mPiecesInited;
};



#endif
