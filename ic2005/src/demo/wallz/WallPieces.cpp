#include "stdafx.h"
#include "WallPieces.h"
//#include <dingus/gfx/DebugRenderer.h>
#include <dingus/math/Plane.h>
#include <dingus/renderer/RenderableBuffer.h>

/*
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

void CWall2D::debugRender( const SVector3* vb, CDebugRenderer& renderer, const TIntVector& pieces )
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
*/


// --------------------------------------------------------------------------


void CWallPiece3D::init( const CWall3D& w, int idx )
{
	const CWallPiece2D& piece = w.getWall2D().getPiece(idx);

	const float HALF_THICK = 0.02f;
	
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
	// remember the count at current state
	mVertsInWall = mVB.size();
	mIndicesInWall = mIB.size();
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


void CWallPiece3D::preRender( int& vbcount, int& ibcount, bool inWall ) const
{
	vbcount = inWall ? mVertsInWall : mVB.size();
	ibcount = inWall ? mIndicesInWall : mIB.size();
}

void CWallPiece3D::render( const SMatrix4x4& matrix, TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount, bool inWall ) const
{
	int i;
	
	// VB
	const SVertexXyzNormal* srcVB = &mVB[0];
	vbcount = inWall ? mVertsInWall : mVB.size();
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
	ibcount = inWall ? mIndicesInWall : mIB.size();
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
	TIntSet			verticesComplex;
	int				borderVertexIndex;
	
	typedef std::map< int, TIntVector > TIntIntsMap;
	TIntIntsMap		vertexNexts;
	TIntIntsMap		vertexPrevs;

	TIntVector		vertexUseCount;

	enum eVertexType {
		VTYPE_NONE,		///< Not computed yet
		VTYPE_INTERIOR,	///< Completely interior
		VTYPE_SINGLE,	///< On the border, belongs to single polygon
		VTYPE_MULTI,	///< On the border, belongs to multiple polygons
		VTYPE_COMPLEX,	///< On the border, belongs to multiple polygons, in a complex polygon forming way
		VTYPE_DONE,		///< Already fully processed (traced into result polygon)
	};
	TIntVector		vertexTypes;


	
	bool isVertexInterior( int idx )
	{
		assert( idx >= 0 && idx < vertexUseCount.size() );

		// vertex is interior if it's use count is >1, AND
		// it's all neighbors' use counts are >1
		assert( vertexUseCount[idx] >= 1 );
		if( vertexUseCount[idx] < 2 )
			return false;

		TIntIntsMap::const_iterator it;
		int i, n;

		it = vertexNexts.find( idx );
		assert( it != vertexNexts.end() );
		const TIntVector& vnext = it->second;
		n = vnext.size();
		for( i = 0; i < n; ++i ) {
			if( vertexUseCount[vnext[i]] < 2 )
				return false;
		}

		it = vertexPrevs.find( idx );
		assert( it != vertexPrevs.end() );
		const TIntVector& vprev = it->second;
		n = vprev.size();
		for( i = 0; i < n; ++i ) {
			if( vertexUseCount[vprev[i]] < 2 )
				return false;
		}

		return true; // fall through: must be interior
	}

	
	bool isVertexComplex( int idx )
	{
		assert( idx >= 0 && idx < vertexUseCount.size() );
		assert( vertexTypes[idx] == VTYPE_NONE );

		// a vertex is simple-shared if of all it's neighbors exactly two
		// vertices are not interior. One non-interior vertex is a bug,
		// all other cases are a complex vertex.
		
		int borderNeighborCount = 0;

		TIntIntsMap::const_iterator it;

		it = vertexNexts.find( idx );
		assert( it != vertexNexts.end() );
		const TIntVector& vnext = it->second;
		int n = vnext.size();
		for( int i = 0; i < n; ++i ) {
			if( vertexTypes[vnext[i]] != VTYPE_INTERIOR )
				++borderNeighborCount;
		}
		assert( borderNeighborCount >= 2 );

		return borderNeighborCount != 2;
	}


	void	markVertexTypes()
	{
		TIntSet::const_iterator vit, vitEnd = vertices.end();

		// first pass: mark all interior and single-border vertices
		for( vit = vertices.begin(); vit != vitEnd; ++vit ) {
			int idx = *vit;
			if( vertexUseCount[idx] == 1 )
				vertexTypes[idx] = VTYPE_SINGLE;
			else if( isVertexInterior(idx) )
				vertexTypes[idx] = VTYPE_INTERIOR;
		}

		// now, the unmarked vertices are all on border: most should be
		// shared, some can form complex polygons
		for( vit = vertices.begin(); vit != vitEnd; ++vit ) {
			int idx = *vit;
			if( vertexTypes[idx] != VTYPE_NONE ) // already marked
				continue;
			borderVertexIndex = idx;
			if( isVertexComplex( idx ) ) {
				vertexTypes[idx] = VTYPE_COMPLEX;
				verticesComplex.insert( idx );
				// just fail now
				assert( false );
				throw 0;
			} else {
				vertexTypes[idx] = VTYPE_MULTI;
			}
		}
	}

	
	void	traceBorder( int idx0, const TWallVertexVector& vb, TIntVector& ib )
	{
		assert( idx0 >= 0 && idx0 < vertexTypes.size() );
		
		TIntVector	polygon;
		polygon.reserve( vertices.size()/2 );

		int idxPrev = idx0;
		int idx = idx0;
		int debugCounter = 0;
		do {
			polygon.push_back( idx );

			// next vertex is the neighbor of current, that is not interior
			// and that is not the previous one
			int idxNext = -1;

			TIntIntsMap::const_iterator it;
			it = vertexNexts.find( idx );
			assert( it != vertexNexts.end() );
			const TIntVector& vnext = it->second;
			int n = vnext.size();
			for( int i = 0; i < n; ++i ) {
				int idx1 = vnext[i];
				if( idx1 != idxPrev && vertexTypes[idx1] != VTYPE_INTERIOR ) {
					idxNext = idx1;
					break;
				}
			}
			assert( idxNext >= 0 );
			idxPrev = idx;
			idx = idxNext;

			assert( ++debugCounter <= vertices.size() );

		} while( idx != idx0 );


		triangulator::process( vb, polygon, ib );
	}


	void	begin( int totalVerts )
	{
		polygons.clear();

		vertices.clear();
		verticesComplex.clear();
		borderVertexIndex = -1;

		vertexNexts.clear();
		vertexPrevs.clear();
		vertexUseCount.resize( 0 );
		vertexUseCount.resize( totalVerts, 0 );
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

	void	end( const TWallVertexVector& vb, TIntVector& ib )
	{
		// mark vertex types
		markVertexTypes();

		// trace and triangulate the polygon
		traceBorder( borderVertexIndex, vb, ib );
	}
};




CWallPieceCombined* CWallPieceCombined::mInitPiece = NULL;
const CWall3D* CWallPieceCombined::mInitWall = NULL;


void CWallPieceCombined::initBegin( const CWall3D& w )
{
	assert( !mInitPiece );
	assert( !mInitWall );
	assert( mVB.empty() );
	assert( mIB.empty() );

	mInitPiece = this;
	mInitWall = &w;
}


void CWallPieceCombined::initAddPiece( int idx )
{
	assert( mInitPiece == this );
	assert( mInitWall );

	const CWallPiece2D& piece = mInitWall->getWall2D().getPiece(idx);

	const float HALF_THICK = 0.02f;
	
	static TIntVector	vertRemap;
	vertRemap.resize(0);
	vertRemap.resize( mInitWall->getWall2D().getVerts().size(), -1 );

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

			SVector2 pos = mInitWall->getWall2D().getVerts()[oldIdx];
			SVector3 pos3( pos.x, pos.y, HALF_THICK );
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
	for( i = 0; i < nverts; ++i ) {
		SVertexXyzNormal vtx = mVB[i];
		vtx.p -= vtx.n * (HALF_THICK*2);
		vtx.n = -vtx.n;
		mVB.push_back( vtx );
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

	// TBD: bounds!
	//mSize.set( piece.getAABB().getSize().x, piece.getAABB().getSize().y, HALF_THICK*2 );
}


void CWallPieceCombined::initEnd()
{
	assert( mInitPiece == this );
	assert( mInitWall );

	mInitPiece = NULL;
	mInitWall = NULL;
}


void CWallPieceCombined::preRender( int& vbcount, int& ibcount ) const
{
	vbcount = mVB.size();
	ibcount = mIB.size();
}

void CWallPieceCombined::render( const SMatrix4x4& matrix, TPieceVertex* vb, unsigned short* ib, int baseIndex, int& vbcount, int& ibcount ) const
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
, mVB(NULL)
, mPieces3D(NULL)
, mFracturedPieces(NULL)
, mLastFractureTime( -100.0f )
, mPiecesInited(false)
, mNeedsRenderingIntoVB(false)
{
	mMatrix.identify();

	for( int i = 0; i < RMCOUNT; ++i )
		mRenderables[i] = NULL;
	
	mRenderables[RM_NORMAL] = new CRenderableIndexedBuffer( NULL, 0 );
	mRenderables[RM_NORMAL]->getParams().setEffect( *RGET_FX("wall") );
	mRenderables[RM_NORMAL]->getParams().addTexture( "tRefl", *RGET_S_TEX(reflTextureID) );
	
	mRenderables[RM_REFLECTED] = new CRenderableIndexedBuffer( NULL, 0 );
	mRenderables[RM_REFLECTED]->getParams().setEffect( *RGET_FX("wallRefl") );
}

CWall3D::~CWall3D()
{
	for( int i = 0; i < RMCOUNT; ++i )
		safeDelete( mRenderables[i] );

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
	mNeedsRenderingIntoVB = true;
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

/*
void CWall3D::debugRender( CDebugRenderer& renderer )
{
	if( !mVB )
		calcVB();
	mWall2D.debugRender( mVB, renderer, mFracturedPieces );
}

void CWall3D::debugRender( CDebugRenderer& renderer, const TIntVector& pieces )
{
	if( !mVB )
		calcVB();
	mWall2D.debugRender( mVB, renderer, pieces );
}
*/

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
	int n = mWall2D.getPieceCount();
	for( i = 0; i < n; ++i ) {
		if( mFracturedPieces[i] )
			continue;
		const CWallPiece3D& pc = mPieces3D[i];
		int nvb, nib;
		pc.preRender( nvb, nib, true );
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
	for( i = 0; i < n; ++i ) {
		if( mFracturedPieces[i] )
			continue;
		int nvb, nib;
		const CWallPiece3D& pc = mPieces3D[i];
		pc.render( pc.getMatrix(), vb, ib, baseIndex, nvb, nib, true );
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
