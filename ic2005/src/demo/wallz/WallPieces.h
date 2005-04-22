#ifndef __WALL_PIECES_H
#define __WALL_PIECES_H

#include <dingus/math/Vector2.h>
#include <dingus/math/Line3.h>
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>
#include <dingus/gfx/geometry/DynamicIBManager.h>
#include "AABox2.h"
#include "Triangulate.h"
#include "../DemoResources.h"


typedef std::vector<SVector2>	TWallVertexVector;
typedef std::vector<int>		TIntVector;

namespace dingus {
	//class CDebugRenderer;
	class CRenderableIndexedBuffer;
};

class CWall3D;


typedef SVertexXyzNormalDiffuse TPieceVertex;
static const DWORD WALL_VERTEX_FVF = FVF_XYZ_NORMAL_DIFFUSE;



// --------------------------------------------------------------------------

/// Single piece of precomputed fracture, sitting in the wall.
class CWallPiece2D {
public:
	CWallPiece2D()
	{
	}
	CWallPiece2D( const CWallPiece2D& r )
		: mPolygon(r.mPolygon)
		, mTris(r.mTris)
		, mAABB(r.mAABB)
	{
	}
	void operator=( const CWallPiece2D& r )
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
		TIntVector	polygonIB;
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
	const TIntVector getPolygonVector() const { return mPolygon; }
	int getVertexCount() const { return mPolygon.size(); }
	const int* getPolygon() const { return &mPolygon[0]; }

	const CAABox2& getAABB() const { return mAABB; }

private:
	TIntVector	mPolygon;	///< Polygon vertex indices of the piece
	TIntVector	mTris;		///< Triangulated piece, 3 indices/tri
	CAABox2		mAABB;		///< 2D AABB of the piece
};



// --------------------------------------------------------------------------


/// Single piece of precomputed fracture, with full 3D representation constructed.
class CWallPiece3D : public boost::noncopyable {
public:
	typedef std::vector<SVertexXyzNormal>	TVertexVector;

public:
	void	init( const CWall3D& w, int idx );
	void	preRender( int& vbcount, int& ibcount, bool inWall ) const;
	void	render( const SMatrix4x4& matrix, TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount, bool inWall ) const;

	const TVertexVector& getVB() const { return mVB; }
	const TIntVector& getIB() const { return mIB; }

	int getVertsInWall() const { return mVertsInWall; }
	int getIndicesInWall() const { return mIndicesInWall; }

	const SMatrix4x4& getMatrix() const { return mMatrix; }
	const SVector3& getSize() const { return mSize; }

private:
	SMatrix4x4		mMatrix; // Initial matrix
	TVertexVector	mVB;
	TIntVector		mIB;
	int				mVertsInWall;
	int				mIndicesInWall;
	SVector3		mSize;
};



// --------------------------------------------------------------------------


/// Piece of precomputed fracture, with in-wall 3D representation constructed.
/// May represent either leaf piece, or combined smaller pieces.
class CWallPieceCombined : public boost::noncopyable {
public:
	typedef std::vector<SVertexXyzNormal>	TVertexVector;

public:
	void	initBegin( const CWall3D& w );
	void	initAddPiece( int idx );
	void	initEnd();

	void	preRender( int& vbcount, int& ibcount ) const;
	void	render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const;

	const TVertexVector& getVB() const { return mVB; }
	const TIntVector& getIB() const { return mIB; }

	const CAABox2& getBounds() const { return mBounds; }

private:
	TVertexVector	mVB;
	TIntVector		mIB;
	CAABox2			mBounds;
	// TBD: quadtree node!
	bool			mLeaf;

	static	CWallPieceCombined*	mInitPiece;
	static	const CWall3D*		mInitWall;
};


// --------------------------------------------------------------------------


/// Single wall with precomputed fracture pieces in 2D.
class CWall2D : public boost::noncopyable {
public:
	CWall2D( const SVector2& size, float smallestElemSize )
		: mSize( size )
		, mSmallestElemSize( smallestElemSize )
	{
	}

	void	addVertex( const SVector2& v ) {
		mVerts.push_back( v );
	}
	const TWallVertexVector& getVerts() const { return mVerts; }

	void	addPiece( const CWallPiece2D& piece ) {
		mPieces.push_back( piece );
	}
	int getPieceCount() const { return mPieces.size(); }
	const CWallPiece2D& getPiece( int i ) const { return mPieces[i]; }

	const SVector2& getSize() const { return mSize; }
	float getSmallestElemSize() const { return mSmallestElemSize; }

	//void	debugRender( const SVector3* vb, CDebugRenderer& renderer, const bool* fractured );
	//void	debugRender( const SVector3* vb, CDebugRenderer& renderer, const TIntVector& pieces );
	
private:
	TWallVertexVector			mVerts;
	std::vector<CWallPiece2D>	mPieces;
	SVector2		mSize;				///< Whole wall spans 0 to mSize range
	float			mSmallestElemSize;
};


// --------------------------------------------------------------------------


/**
 *	Single wall with precomputed fracture pieces, positioned in 3D and
 *  tracking fractured out pieces' state.
 */ 
class CWall3D : public boost::noncopyable {
public:
	CWall3D( const SVector2& size, float smallestElemSize, const char* reflTextureID );
	~CWall3D();

	const CWall2D& getWall2D() const { return mWall2D; }
	CWall2D& getWall2D() { return mWall2D; }

	const CWallPiece3D* getPieces3D() const { return mPieces3D; }

	const SMatrix4x4& getMatrix() const { return mMatrix; }
	void setMatrix( const SMatrix4x4& m ) { mMatrix = m; mInvMatrix = m; mInvMatrix.invert(); }

	void	calcVB();

	void	update( float t );

	bool	intersectRay( const SLine3& ray, float& t ) const;
	void	fracturePiecesInSphere( float t, bool fractureOut, const SVector3& pos, float radius, TIntVector& pcs );

	//void	debugRender( CDebugRenderer& renderer );
	//void	debugRender( CDebugRenderer& renderer, const TIntVector& pieces );

	void	render( eRenderMode rm );

private:
	void	initPieces();
	bool	renderIntoVB();

private:
	CWall2D		mWall2D;

	SMatrix4x4	mMatrix;
	SMatrix4x4	mInvMatrix;
	SVector3*	mVB;

	CWallPiece3D*	mPieces3D;
	bool*			mFracturedPieces;
	float			mLastFractureTime;

	std::vector<CWallPieceCombined*>	mPiecesCombined;

	CRenderableIndexedBuffer*	mRenderables[RMCOUNT];
	TVBChunk::TSharedPtr		mVBChunk;
	TIBChunk::TSharedPtr		mIBChunk;

	bool	mPiecesInited;
	bool	mNeedsRenderingIntoVB;
};



#endif
