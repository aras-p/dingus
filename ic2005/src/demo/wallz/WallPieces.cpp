#include "stdafx.h"
#include "WallPieces.h"
#include <dingus/math/Plane.h>
#include <dingus/math/MathUtils.h>
#include <dingus/renderer/RenderableBuffer.h>


const float HALF_THICK = 0.02f;



static inline float signedAngle2D( const SVector2& a, const SVector2& b )
{
	float sgn = a.x*b.y > b.x*a.y ? 1.0f : -1.0f;
	float dotp = a.dot( b ) / (a.length()*b.length());
	dotp = clamp( dotp, -1.0f, 1.0f );
	float ang = acosf( dotp );
	return sgn * ang;
}

// --------------------------------------------------------------------------


void CWallPiece3D::init( const CWall3D& w, int idx )
{
	const CWallPiece2D& piece = w.getWall2D().getPiece(idx);

	// construct VB/IB for this piece, with positions centered
	SVector2 pcenter = piece.getAABB().getCenter();

	static TIntVector	vertRemap;
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
			vtx.p.set( pos.x, pos.y, HALF_THICK );
			vtx.n.set( 0, 0, 1 );
			mVB.push_back( vtx );
		}
		mIB.push_back( newIdx );
	}
	for( i = 0; i < nidx/3; ++i ) {
		int iii = mIB[i*3+1];
		mIB[i*3+1] = mIB[i*3+2];
		mIB[i*3+2] = iii;
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
		v0.n = v1.n = v2.n = v3.n = -normal;
		mVB.push_back( v0 );
		mVB.push_back( v1 );
		mVB.push_back( v2 );
		mVB.push_back( v3 );
		mIB.push_back( nverts*2 + i*4 + 0 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 2 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 3 );
		mIB.push_back( nverts*2 + i*4 + 2 );
	}

	// construct initial mMatrix
	mMatrix.identify();
	mMatrix = w.getMatrix();
	mMatrix.getOrigin() += mMatrix.getAxisX() * pcenter.x;
	mMatrix.getOrigin() += mMatrix.getAxisY() * pcenter.y;

	mSize.set( piece.getAABB().getSize().x, piece.getAABB().getSize().y, HALF_THICK*2 );
}


void CWallPiece3D::preRender( int& vbcount, int& ibcount ) const
{
	vbcount = mVB.size();
	ibcount = mIB.size();
}

void CWallPiece3D::render( const SMatrix4x4& matrix, TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
{
	int i;
	
	// VB
	const SVertexXyzNormal* srcVB = &mVB[0];
	vbcount = mVB.size();
	for( i = 0; i < vbcount; ++i ) {
		SVector3 p, n;
		D3DXVec3TransformCoord( &p, &srcVB->p, &matrix );
		D3DXVec3TransformNormal( &n, &srcVB->n, &matrix );
		vb->p = p;
		vb->n = n;
		vb->diffuse = 0xFFff8000; // TBD
		++srcVB;
		++vb;
	}

	// IB
	const int* srcIB = &mIB[0];
	ibcount = mIB.size();
	for( i = 0; i < ibcount; ++i ) {
		int idx = *srcIB + baseIndex;
		assert( idx >= 0 && idx < 64000 );
		*ib = idx;
		++srcIB;
		++ib;
	}
}


// --------------------------------------------------------------------------


namespace polygon_merger {

	std::vector<TIntVector>	polygons;

	typedef std::set<int>	TIntSet;
	TIntSet			vertices;
	int				borderVertexIndex;
	
	typedef std::map< int, TIntVector > TIntIntsMap;
	TIntIntsMap		vertexNexts;
	TIntIntsMap		vertexPrevs;

	TIntVector		vertexUseCount;
	TIntVector		vertexTraceID;

	enum eVertexType {
		VTYPE_NONE,		///< Not computed yet
		VTYPE_INTERIOR,	///< Completely interior
		VTYPE_SINGLE,	///< On the border, belongs to single polygon
		VTYPE_MULTI,	///< On the border, belongs to multiple polygons
		VTYPE_DONE,		///< Already fully processed (traced into result polygon)
	};
	TIntVector		vertexTypes;


	
	bool isVertexInterior( int idx )
	{
		assert( idx >= 0 && idx < vertexUseCount.size() );

		// vertex is interior if it's use count is >1, AND
		// it's all neighbors' use counts are >1, AND it has exactly "usecount"
		// different neighbors.
		int useCount = vertexUseCount[idx];
		assert( useCount >= 1 );
		if( useCount < 2 )
			return false;

		TIntIntsMap::const_iterator it;
		int i, n;

		it = vertexNexts.find( idx );
		assert( it != vertexNexts.end() );
		const TIntVector& vnext = it->second;
		n = vnext.size();
		assert( n == useCount );
		for( i = 0; i < n; ++i ) {
			if( vertexUseCount[vnext[i]] < 2 )
				return false;
		}

		it = vertexPrevs.find( idx );
		assert( it != vertexPrevs.end() );
		const TIntVector& vprev = it->second;
		n = vprev.size();
		assert( n == useCount );
		for( i = 0; i < n; ++i ) {
			int nidx = vprev[i];
			if( vertexUseCount[nidx] < 2 )
				return false;
			if( std::find( vnext.begin(), vnext.end(), nidx ) == vnext.end() )
				return false;
		}

		return true; // fall through: must be interior
	}



	void	markVertexTypes()
	{
		TIntSet::const_iterator vit, vitEnd = vertices.end();

		// first pass: mark all interior and single-border vertices
		for( vit = vertices.begin(); vit != vitEnd; ++vit ) {
			int idx = *vit;
			if( vertexUseCount[idx] == 1 ) {
				vertexTypes[idx] = VTYPE_SINGLE;
				borderVertexIndex = idx;
			} else if( isVertexInterior(idx) )
				vertexTypes[idx] = VTYPE_INTERIOR;
		}

		// now, the unmarked vertices are all shared and on border
		for( vit = vertices.begin(); vit != vitEnd; ++vit ) {
			int idx = *vit;
			if( vertexTypes[idx] == VTYPE_NONE ) {
				vertexTypes[idx] = VTYPE_MULTI;
				borderVertexIndex = idx;
			}
		}
	}

	
	void	traceBorder( int idx0, const TWallVertexVector& vb, TIntVector& polygon, TIntVector& ib )
	{
		static int traceID = 0;
		++traceID;

		assert( idx0 >= 0 && idx0 < vertexTypes.size() );
		
		ib.resize( 0 );
		polygon.resize( 0 );
		polygon.reserve( vertices.size()/2 );

		TIntVector localPolygon;
		localPolygon.reserve( 128 );
		TIntVector localIB;
		localIB.reserve( 128 );

		int idxPrev = idx0;
		int idx = idx0;
		int debugCounter = 0;

		int debugLoopCounter = 0;

		do {

			localIB.resize( 0 );

			do{

				localPolygon.push_back( idx );
				vertexTraceID[idx] = traceID;
				assert( ++debugCounter <= vertices.size()*2 );

				// Next vertex is the neighbor of current, that is not interior
				// and that is not the previous one.
				// When there are many possible ones, trace based on angle.

				int idxNext = -1;

				SVector2 prevToCurr = vb[idx] - vb[idxPrev];
				if( prevToCurr.lengthSq() < 1.0e-6f )
					prevToCurr.set( 0.01f, 0.01f );

				TIntIntsMap::const_iterator it;
				it = vertexNexts.find( idx );
				assert( it != vertexNexts.end() );
				const TIntVector& vnext = it->second;
				int n = vnext.size();
				float bestAngle = 100.0f;
				for( int i = 0; i < n; ++i ) {
					int idx1 = vnext[i];
					if( idx1 != idxPrev && vertexTypes[idx1] != VTYPE_INTERIOR ) {
						SVector2 currToNext = vb[idx1] - vb[idx];
						float ang = signedAngle2D( prevToCurr, currToNext );
						if( ang < bestAngle ) {
							bestAngle = ang;
							idxNext = idx1;
						}
					}
				}
				assert( bestAngle > -4.0f && bestAngle < 4.0f );
				assert( idxNext >= 0 );
				idxPrev = idx;
				idx = idxNext;

			} while( vertexTraceID[idx] != traceID );

			assert( localPolygon.size() >= 3 );
			//if( localPolygon.size() < 3 ) {
			//	return;
			//}

			assert( ++debugLoopCounter < vertices.size() );

			if( idx == idx0 ) {
				// The polygon is simple or we found the last loop.
				// Triangulate local and append to results.
				triangulator::process( vb, localPolygon, localIB );
				polygon.insert( polygon.end(), localPolygon.begin(), localPolygon.end() );
				ib.insert( ib.end(), localIB.begin(), localIB.end() );
				return;
			}

			// The polygon must be complex, and we just found a closed loop.
			// Take only the loop, triangulate it, append to results, continue.
			TIntVector::const_iterator itLoopStart = 
				std::find( localPolygon.begin(), localPolygon.end(), idx );
			assert( itLoopStart != localPolygon.end() );

			// append to results
			TIntVector loopPolygon( itLoopStart, localPolygon.end() );
			triangulator::process( vb, loopPolygon, localIB );
			polygon.insert( polygon.end(), loopPolygon.begin(), loopPolygon.end() );
			ib.insert( ib.end(), localIB.begin(), localIB.end() );

			// continue - remove the looped polygon from local
			localPolygon.resize( itLoopStart - localPolygon.begin() );

		} while( true );
	}


	void	begin( int totalVerts )
	{
		polygons.clear();

		vertices.clear();
		borderVertexIndex = -1;

		vertexNexts.clear();
		vertexPrevs.clear();
		vertexUseCount.resize( 0 );
		vertexUseCount.resize( totalVerts, 0 );

		vertexTraceID.resize( totalVerts, -1 );

		vertexTypes.resize( 0 );
		vertexTypes.resize( totalVerts, VTYPE_NONE );
	}

	void	addPolygon( const TIntVector& ib )
	{
		polygons.push_back( ib );

		TIntVector::const_iterator vit, vitEnd = ib.end();
		for( vit = ib.begin(); vit != vitEnd; ++vit ) {
			int idx = *vit;
			assert( idx >= 0 && idx < vertexUseCount.size() );
			vertices.insert( idx );
			++vertexUseCount[idx];
			int idxNext = (vit==vitEnd-1) ? ib.front() : *(vit+1);
			int idxPrev = (vit==ib.begin()) ? ib.back() : *(vit-1);
			vertexNexts[idx].push_back( idxNext );
			vertexPrevs[idx].push_back( idxPrev );
		}
	}

	void	end( const TWallVertexVector& vb, TIntVector& polygon, TIntVector& ib )
	{
		if( polygons.size() == 1 ) {
			
			// trivial case, just output input polygon
			polygon = polygons[0];
			triangulator::process( vb, polygon, ib );

		} else {
			
			// mark vertex types
			markVertexTypes();

			// trace and triangulate the polygon(s)
			traceBorder( borderVertexIndex, vb, polygon, ib );
		}
	}

}; // namespace polygon_merger




// --------------------------------------------------------------------------



CWallPieceCombined* CWallPieceCombined::mInitPiece = NULL;
const CWall3D* CWallPieceCombined::mInitWall = NULL;



void CWallPieceCombined::initBegin( const CWall3D& w, TWallQuadNode* quadnode )
{
	assert( !mInitPiece );
	assert( !mInitWall );
	assert( mVB.empty() );
	assert( mIB.empty() );

	mInitPiece = this;
	mInitWall = &w;

	mQuadNode = quadnode;

	polygon_merger::begin( w.getWall2D().getVerts().size() );
}


void CWallPieceCombined::initAddPiece( int idx )
{
	const CWallPiece2D& piece = mInitWall->getWall2D().getPiece(idx);
	polygon_merger::addPolygon( piece.getPolygonVector() );

	mCombinedPieces.push_back( idx );

	mBounds.extend( piece.getAABB() );
}


void CWallPieceCombined::initEnd( TWallQuadNode* quadtree )
{
	assert( mInitPiece == this );
	assert( mInitWall );

	const TWallVertexVector& wallVerts = mInitWall->getWall2D().getVerts();

	// merge the polygons
	TIntVector polygon;
	TIntVector ibFront;
	polygon_merger::end( wallVerts, polygon, ibFront );


	static TIntVector	vertRemap;
	vertRemap.resize(0);
	vertRemap.resize( wallVerts.size(), -1 );

	int i;

	int nidx = ibFront.size();
	mIB.reserve( nidx + polygon.size()*6 );
	mVB.reserve( polygon.size()*5 );

	// construct the front side
	for( i = 0; i < nidx; ++i ) {
		int oldIdx = ibFront[i];
		int newIdx = vertRemap[oldIdx];
		if( newIdx < 0 ) {
			newIdx = mVB.size();
			vertRemap[oldIdx] = newIdx;

			SVector2 pos = wallVerts[oldIdx];
			SVector3 pos3( pos.x, pos.y, 0.0f );
			D3DXVec3TransformCoord( &pos3, &pos3, &mInitWall->getMatrix() );
			SVertexXyzNormal vtx;
			vtx.p = pos3;
			vtx.n = mInitWall->getMatrix().getAxisZ();
			mVB.push_back( vtx );
		}
		mIB.push_back( newIdx );
	}
	for( i = 0; i < nidx/3; ++i ) {
		int iii = mIB[i*3+1];
		mIB[i*3+1] = mIB[i*3+2];
		mIB[i*3+2] = iii;
	}
	// push vertices for another side (but no triangles)
	int nverts = mVB.size();
	int npolygon = polygon.size();
	for( i = 0; i < nverts; ++i ) {
		SVertexXyzNormal vtx = mVB[i];
		vtx.p -= vtx.n * (HALF_THICK*2);
		vtx.n = -vtx.n;
		mVB.push_back( vtx );
	}
	// construct side caps
	for( i = 0; i < nverts; ++i ) {
		int oldIdx0 = polygon[i];
		int oldIdx1 = polygon[(i+1)%npolygon];
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
		v0.n = v1.n = v2.n = v3.n = -normal;
		mVB.push_back( v0 );
		mVB.push_back( v1 );
		mVB.push_back( v2 );
		mVB.push_back( v3 );
		mIB.push_back( nverts*2 + i*4 + 0 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 2 );
		mIB.push_back( nverts*2 + i*4 + 1 );
		mIB.push_back( nverts*2 + i*4 + 3 );
		mIB.push_back( nverts*2 + i*4 + 2 );
	}


	if( !mQuadNode ) {
		
		assert( mCombinedPieces.size()==1 );

		TWallQuadNode* node = quadtree->getNode( mBounds );
		assert( node );
		while( node ) {
			node->getData().leafs.push_back( this );
			node = node->getParent();
		}

	} else {

		// record ourselves in the quadtree
		assert( !mQuadNode->getData().combined );
		mQuadNode->getData().combined = this;
	}


	mInitPiece = NULL;
	mInitWall = NULL;
}


void CWallPieceCombined::preRender( int& vbcount, int& ibcount ) const
{
	vbcount = mVB.size();
	ibcount = mIB.size();
}

void CWallPieceCombined::render( TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
{
	int i;
	
	// VB
	const SVertexXyzNormal* srcVB = &mVB[0];
	vbcount = mVB.size();
	for( i = 0; i < vbcount; ++i ) {
		vb->p = srcVB->p;
		vb->n = srcVB->n;
		vb->diffuse = 0xFFff8000; // TBD
		++srcVB;
		++vb;
	}

	// IB
	const int* srcIB = &mIB[0];
	ibcount = mIB.size();
	for( i = 0; i < ibcount; ++i ) {
		int idx = *srcIB + baseIndex;
		assert( idx >= 0 && idx < 64000 );
		*ib = idx;
		++srcIB;
		++ib;
	}
}


// --------------------------------------------------------------------------


CWall3D::CWall3D( const SVector2& size, float smallestElemSize, const char* reflTextureID )
: mWall2D(size,smallestElemSize)
, mPieces3D(NULL)
, mFracturedPieces(NULL)
, mQuadtree(NULL)
, mLastFractureTime( -100.0f )
, mPiecesInited(false)
, mNeedsRenderingIntoVB(false)
{
	mMatrix.identify();

	for( int i = 0; i < RMCOUNT; ++i )
		mRenderables[i] = NULL;
	
	mRenderables[RM_NORMAL] = new CRenderableIndexedBuffer( NULL, 0 );
	mRenderables[RM_NORMAL]->getParams().setEffect( *RGET_FX("wall") );
	if( reflTextureID )
		mRenderables[RM_NORMAL]->getParams().addTexture( "tRefl", *RGET_S_TEX(reflTextureID) );
	
	mRenderables[RM_REFLECTED] = new CRenderableIndexedBuffer( NULL, 0 );
	mRenderables[RM_REFLECTED]->getParams().setEffect( *RGET_FX("wallRefl") );
}

CWall3D::~CWall3D()
{
	for( int i = 0; i < RMCOUNT; ++i )
		safeDelete( mRenderables[i] );

	stl_utils::wipe( mPiecesCombined );
	safeDeleteArray( mPieces3D );
	safeDeleteArray( mFracturedPieces );

	safeDeleteArray( mQuadtree );
}

void CWall3D::initPieces()
{
	assert( !mPiecesInited );
	assert( !mFracturedPieces );
	assert( !mPieces3D );
	assert( mPiecesCombined.empty() );
	
	
	const int QUADTREE_DEPTH = 3;
	mQuadtree = TWallQuadNode::create( SVector2(0,0), mWall2D.getSize(), QUADTREE_DEPTH, &mQuadtreeNodeCount );

	int i;
	int n = mWall2D.getPieceCount();

	mPieces3D = new CWallPiece3D[n];
	mFracturedPieces = new bool[n];

	// init the leaf pieces
	for( i = 0; i < n; ++i ) {
		mPieces3D[i].init( *this, i );
		mFracturedPieces[i] = false;

		CWallPieceCombined* wpc = new CWallPieceCombined();
		wpc->initBegin( *this, NULL );
		wpc->initAddPiece( i );
		wpc->initEnd( mQuadtree );
		mPiecesCombined.push_back( wpc );
	}

	// go through quadtree nodes and merge the pieces
	n = mQuadtreeNodeCount;
	for( i = 0; i < n; ++i ) {
		TWallQuadNode& node = mQuadtree[i];
		assert( !node.getData().combined );
		CWallPieceCombined* wpc = new CWallPieceCombined();
		wpc->initBegin( *this, &node );
		int npc = node.getData().leafs.size();
		for( int j = 0; j < npc; ++j )
			wpc->initAddPiece( node.getData().leafs[j]->getLeafIndex() );
		wpc->initEnd( mQuadtree );

		mPiecesCombined.push_back( wpc );
	}

	mPiecesInited = true;
	mNeedsRenderingIntoVB = true;
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


void CWall3D::fracturePiecesInSphere( float t, bool fractureOut, const SVector3& pos, float radius, TIntVector& pcs )
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
			if( fractureOut ) {
				mFracturedPieces[i] = true;
				mNeedsRenderingIntoVB = true;
			}
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


bool CWall3D::renderIntoVB()
{
	mNeedsRenderingIntoVB = false;

	int nverts = 0, nindices = 0;
	mVBChunk = NULL;
	mIBChunk = NULL;

	int i;

	// accumulate total vertex/index count
	//int n = mPiecesCombined.size();
	//int n = mQuadtree[1].getData().leafs.size();
	//for( i = 0; i < n; ++i ) {
	{
		//if( mFracturedPieces[i] )
		//	continue;
		//const CWallPieceCombined& pc = *mQuadtree[1].getData().leafs[i];
		const CWallPieceCombined& pc = *mQuadtree[1].getData().combined;
		int nvb, nib;
		pc.preRender( nvb, nib );
		nverts += nvb;
		nindices += nib;
	}

	if( !nverts || !nindices )
		return false;

	// lock and render
	assert( nindices < 64000 );
	mVBChunk = CDynamicVBManager::getInstance().allocateChunk( nverts, sizeof(TPieceVertex) );
	mIBChunk = CDynamicIBManager::getInstance().allocateChunk( nindices, 2 );
	TPieceVertex* vb = (TPieceVertex*)mVBChunk->getData();
	unsigned short* ib = (unsigned short*)mIBChunk->getData();
	int baseIndex = 0;
	//for( i = 0; i < n; ++i ) {
	{
		//if( mFracturedPieces[i] )
		//	continue;
		int nvb, nib;
		//const CWallPieceCombined& pc = *mQuadtree[1].getData().leafs[i];
		const CWallPieceCombined& pc = *mQuadtree[1].getData().combined;
		pc.render( vb, ib, baseIndex, nvb, nib );
		baseIndex += nvb;
		vb += nvb;
		ib += nib;
	}
	mVBChunk->unlock();
	mIBChunk->unlock();

	// setup renderables
	for( i = 0; i < RMCOUNT; ++i ) {
		if( !mRenderables[i] )
			continue;
		mRenderables[i]->resetVBs();

		mRenderables[i]->setVB( mVBChunk->getBuffer(), 0 );
		mRenderables[i]->setStride( mVBChunk->getStride(), 0 );
		mRenderables[i]->setBaseVertex( mVBChunk->getOffset() );
		mRenderables[i]->setMinVertex( 0 );
		mRenderables[i]->setNumVertices( mVBChunk->getSize() );

		mRenderables[i]->setIB( mIBChunk->getBuffer() );
		mRenderables[i]->setStartIndex( mIBChunk->getOffset() );
		mRenderables[i]->setPrimCount( mIBChunk->getSize() / 3 );

		mRenderables[i]->setPrimType( D3DPT_TRIANGLELIST );
	}
	return true;
}


void CWall3D::render( eRenderMode rm )
{
	if( !mRenderables[rm] )
		return;

	if( mNeedsRenderingIntoVB ||
		!mVBChunk || !mVBChunk->isValid() ||
		!mIBChunk || !mIBChunk->isValid() )
	{
		renderIntoVB();
	}
	G_RENDERCTX->attach( *mRenderables[rm] );
}
