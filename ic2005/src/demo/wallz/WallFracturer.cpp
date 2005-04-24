#include "stdafx.h"
#include "WallFracturer.h"
#include "WallPieces.h"
#include "Triangulate.h"
#include <dingus/utils/CpuTimer.h>

#include <dingus/utils/Random.h>


static const int FRAC_RANDOM_SEED = 64827;
static CRandom	gFracRandom;
static bool		gFracRandomInited = false;


// --------------------------------------------------------------------------


class CFracGrid : public boost::noncopyable {
public:
	struct SElement {
		int		id;
		int		size;
	};
	enum { MAX_COMBINED_SIZE = 5 };

public:
	CFracGrid( int rows, int cols, const SVector2& size );
	~CFracGrid();

	int		getRows() const { return mRows; }
	int		getCols() const { return mCols; }

	const SVector2& getNode( int col, int row ) const {
		return mNodes[row*mCols+col];
	}

	const SElement& getElem( int col, int row ) const {
		return mElems[row*mElCols+col];
	}

	void	distort();
	void	combine();
	void	toPieces( CWall2D& pieces );


private:
	void changeNodePos( int node, const SVector2& pos ) {
		// TBD: more tests needed, now nodes may be turned into non convex ones
		if( checkConvex(pos, node-1, node-mCols) &&
			checkConvex(pos, node-mCols, node+1) &&
			checkConvex(pos, node+1, node+mCols) &&
			checkConvex(pos, node+mCols, node-1) )
		{
			mNodes[node] = pos;
		}
	}

	bool checkConvex( const SVector2& pos, int na, int nb ) const {
		SVector2 toa = mNodes[na] - pos;
		SVector2 tob = mNodes[nb] - pos;
		toa.normalize();
		tob.normalize();
		bool convex = ( toa.x * tob.y - toa.y * tob.x > D3DX_PI/40 );
		return convex;
	}

	void	elemToPiece( int er, int ec, int eidx, CWall2D& pieces, int* vertIdxInPieces );

	int		tryTraceCluster( int vidx, int dir, int elID ) const;

private:
	int			mRows, mCols;
	SVector2	mSize;
	SVector2*	mNodes;

	int			mElRows, mElCols;
	SElement*	mElems;
};


CFracGrid::CFracGrid( int rows, int cols, const SVector2& size )
: mRows( rows ), mCols( cols ), mSize( size )
, mElRows( rows-1 ), mElCols( cols-1 )
{
	int r, c, k;

	mNodes = new SVector2[ cols * rows ];
	k = 0;
	for( r = 0; r < rows; ++r ) {
		for( c = 0; c < cols; ++c ) {
			mNodes[k].set( size.x*c/(cols-1), size.y*r/(rows-1) );
			++k;
		}
	}

	mElems = new SElement[ mElCols * mElRows ];
	k = 0;
	for( r = 0; r < mElRows; ++r ) {
		for( c = 0; c < mElCols; ++c ) {
			mElems[k].id = k;
			mElems[k].size = 1;
			++k;
		}
	}
}

CFracGrid::~CFracGrid()
{
	delete[] mNodes;
	delete[] mElems;
}


void CFracGrid::distort() {
	const float xmax = mSize.x/mCols * 0.15f;
	const float ymax = mSize.y/mRows * 0.15f;

	int n = mRows * mCols;
	for( int i = 0; i < n; ++ i ) {
		int r = gFracRandom.getInt( 1, mRows-1 );
		int c = gFracRandom.getInt( 1, mCols-1 );
		int idx = r*mCols+c;
		SVector2 pos = mNodes[idx] + SVector2(
			gFracRandom.getFloat( -xmax, xmax ),
			gFracRandom.getFloat( -ymax, ymax )
		);
		changeNodePos( idx, pos );
	}
}


void CFracGrid::combine()
{
	int n = mElRows * mElCols;

	int cmbn = n * 5;
	for( int i = 0; i < cmbn; ++i ) {
		int r = gFracRandom.getInt( 1, mElRows-1 );
		assert( r >= 1 && r < mElRows-1 );
		int c = gFracRandom.getInt( 1, mElCols-1 );
		assert( c >= 1 && c < mElCols-1 );

		int idx = r*mElCols+c;
		SElement& el = mElems[idx];

		// element is large already, don't combine
		assert( el.size >= 0 );
		if( el.size >= MAX_COMBINED_SIZE )
			continue;

		// attach 2 elements to this
		for( int z = 0; z < 2; ++z ) {
			int dir = gFracRandom.getInt() & 3;
			static int dr[4] = { -1, 0, 1, 0 };
			static int dc[4] = { 0, -1, 0, 1 };
			int idx2 = idx + dr[dir]*mElCols + dc[dir];
			assert( idx2 >= 0 && idx2 < mElCols*mElRows );

			// if the neighbor is already us, then don't combine
			SElement& el2 = mElems[idx2];
			if( el2.id == el.id )
				continue;

			// if the neighbor plus this sizes are large, then don't combine
			int newSize = el2.size + el.size;
			if( newSize > MAX_COMBINED_SIZE )
				continue;
			assert( newSize >= 1 && newSize <= MAX_COMBINED_SIZE );

			// combine
			int oldId = el2.id;

			// go through region around initial element
			// and add elems to out cluster.
			int er0 = r - MAX_COMBINED_SIZE; if( er0 < 0 ) er0 = 0;
			int er1 = r + MAX_COMBINED_SIZE+1; if( er1 > mElRows ) er1 = mElRows;
			int ec0 = c - MAX_COMBINED_SIZE; if( ec0 < 0 ) ec0 = 0;
			int ec1 = c + MAX_COMBINED_SIZE+1; if( ec1 > mElCols ) ec1 = mElCols;
			for( int ir = er0; ir < er1; ++ir ) {
				for( int ic = ec0; ic < ec1; ++ic ) {
					SElement& e = mElems[ir*mElCols+ic];
					if( e.id == oldId )
						e.id = el.id;
					if( e.id == el.id )
						e.size = newSize;
				}
			}
			// haha, stupid linear search
			/*
			for( int j = 0; j < n; ++j ) {
				SElement& e = mElems[j];
				if( e.id == oldId )
					e.id = el.id;
				if( e.id == el.id )
					e.size = newSize;
			}
			*/
		}
	}
}

void CFracGrid::toPieces( CWall2D& pieces )
{
	// Here we have a regular 2d grid of elements. 4-way adjacent clusters
	// of them may be combined, each cluster has unique non-negative ID.
	
	// Some of the inner vertices of the combined cluster may be thrown out.
	// So, not all verts will be added to pieces. Have a
	// index -> index_in_pieces map here (-1 means that vertex is thrown out).
	int* vertIdxInPieces = new int[ mCols * mRows ];
	memset( vertIdxInPieces, -1, mCols*mRows*sizeof(vertIdxInPieces[0]) );

	// Go through all elements and turn it's cluster into piece. Set cluster's
	// ID to -1 so that it's not touched again. Fetch vertices into pieces
	// as needed.
	int eidx = 0;
	for( int er = 0; er < mElRows; ++er ) {
		for( int ec = 0; ec < mElCols; ++ec, ++eidx ) {
			SElement& elem = mElems[eidx];
			if( elem.id < 0 )
				continue; // the element is already turned into piece
			elemToPiece( er, ec, eidx, pieces, vertIdxInPieces );
		}
	}

	delete[] vertIdxInPieces;
}

#define VTX_R(idx) (idx/mCols)
#define VTX_C(idx) (idx%mCols)
#define ELEM_R(idx) (idx/mElCols)
#define ELEM_C(idx) (idx%mElCols)


/**
 *  Return the traced-to vertex index.
 *  If tracing in this direction would not be the current cluster,
 *  returns -1.
 */
int CFracGrid::tryTraceCluster( int vidx, int dir, int elID ) const
{
	assert( vidx >= 0 && vidx < mCols * mRows );
	assert( dir >= 0 && dir < 4 );

	// 4 directions, clockwise: right, down, left, up
	const int dirDR[4] = { 0, 1, 0, -1 }; // delta row
	const int dirDC[4] = { 1, 0, -1, 0 }; // delta col

	// see if we can go in that direction
	int vr = VTX_R(vidx);
	int vc = VTX_C(vidx);
	// on 0th row we don't want to go left, etc.
	if( vr==0 && dir==2 ||
		vc==mCols-1 && dir==3 ||
		vr==mRows-1 && dir==0 ||
		vc==0 && dir==1 )
	{
		return -1;
	}
	// maybe we'll be out of bounds in there?
	int vr2 = vr + dirDR[dir];
	if( vr2 < 0 || vr2 >= mRows )
		return -1;
	int vc2 = vc + dirDC[dir];
	if( vc2 < 0 || vc2 >= mCols )
		return -1;

	// get the element that's to the right of vidx -> vidx+dir
	const int EL_IDX_FIX[4] = { 0, -1, -mElCols-1, -mElCols };
	int eidx = vr*mElCols + vc + EL_IDX_FIX[dir];
	assert( eidx >= 0 && eidx < mElCols*mElRows );

	// check if it's ID matches the one we're after
	if( mElems[eidx].id != elID )
		return -1;

	return vr2*mCols + vc2;
}


void CFracGrid::elemToPiece( int er, int ec, int eidx, CWall2D& pieces, int* vertIdxInPieces )
{
	// The initial element is always upper left corner of the cluster.
	// So fetch it's UL vertex and start traversing to the right.

	int vidx = er * mCols + ec;
	int curDir = 0; // right
	const int v0idx = vidx; // starting vertex index
	const int elID = mElems[eidx].id; // cluster's ID

	// walk the boundary of the cluster and fetch in the needed vertices
	int edgeCounter = 0;
	CWallPiece2D pc;
	do {
		assert( vidx >= 0 && vidx < mCols*mRows );
		// add current vertex to pieces if not added yet
		if( vertIdxInPieces[vidx] < 0 ) {
			vertIdxInPieces[vidx] = pieces.getVerts().size();
			pieces.addVertex( mNodes[vidx] );
		}
		// add vertex index to piece
		pc.addVertex( vertIdxInPieces[vidx] );

		// walk to next vertex
		int vidx2;
		int rotations = 0;
		while( (vidx2 = tryTraceCluster(vidx, curDir, elID)) < 0 ) {
			// rotate clockwise
			curDir = (curDir+1) & 3;
			++rotations;
			assert( rotations < 4 );
		}

		// go to next vertex
		vidx = vidx2;

		// next walk direction is to the left of current direction
		curDir = (curDir-1) & 3;

		// we certainly can't have more edges than all edges of max possible elements
		++edgeCounter;
		assert( edgeCounter < MAX_COMBINED_SIZE*4 );
		
	} while( vidx != v0idx );

	// Triangulate and add the piece
	pc.finish( pieces.getVerts() );
	pieces.addPiece( pc );

	// go through region around initial element
	// and set our cluster's elements as processed. A bit slow, but who cares :)
	int er0 = er - MAX_COMBINED_SIZE; if( er0 < 0 ) er0 = 0;
	int er1 = er + MAX_COMBINED_SIZE+1; if( er1 > mElRows ) er1 = mElRows;
	int ec0 = ec - MAX_COMBINED_SIZE; if( ec0 < 0 ) ec0 = 0;
	int ec1 = ec + MAX_COMBINED_SIZE+1; if( ec1 > mElCols ) ec1 = mElCols;
	for( int r = er0; r < er1; ++r ) {
		for( int c = ec0; c < ec1; ++c ) {
			int idx = r*mElCols+c;
			if( mElems[idx].id == elID )
				mElems[idx].id = -1;
		}
	}
}




// --------------------------------------------------------------------------


void wallFractureCompute( CWall2D& wall )
{
	if( !gFracRandomInited ) {
		gFracRandom.seed( FRAC_RANDOM_SEED );
		gFracRandomInited = true;
	}

	CFracGrid* fracGrid = 0;
	int cols = wall.getSize().x / wall.getSmallestElemSize();
	int rows = wall.getSize().y / wall.getSmallestElemSize();

	// init
	{
		cputimer::debug_interval ttt( "frac create" );
		fracGrid = new CFracGrid( rows, cols, wall.getSize() );
	}
	// distort
	{
		cputimer::debug_interval ttt( "frac distort" );
		const int DISTORT_ITERS = 6;
		for( int i = 0; i < DISTORT_ITERS; ++i )
			fracGrid->distort();
	}

	// combine
	{
		cputimer::debug_interval ttt( "frac distort" );
		fracGrid->combine();
	}

	// turn into pieces
	{
		cputimer::debug_interval ttt( "frac to pieces" );
		fracGrid->toPieces( wall );
	}

	delete fracGrid;
}
