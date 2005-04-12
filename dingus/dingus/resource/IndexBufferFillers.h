// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __IB_FILLERS_H
#define __IB_FILLERS_H

#include "IndexBufferBundle.h"

namespace dingus {

/**
 *  Index buffer filler for separate quads.
 *
 *  Fills 6 indices for a quad in this way:
 *		0----13
 *		|   / |
 *		| /   |
 *		25----4
 *  Here vertex 0 is upper left, 1 is upper right, 2 is lower right, 3 is lower left (so clockwise).
 */
class CIBFillerQuads : public IIndexBufferFiller {
public:
	virtual void fillIB( CD3DIndexBuffer& ib, int elements, D3DFORMAT format ) {
		assert( format == D3DFMT_INDEX16 );
		assert( elements % 6 == 0 );
		int n = elements / 6;
		unsigned short* pib = NULL;
		ib.getObject()->Lock( 0, elements*2, (void**)&pib, 0 );
		assert( &ib );
		for( int i = 0; i < n; ++i ) {
			unsigned short base = i * 4;
			pib[0] = base;
			pib[1] = base+1;
			pib[2] = base+3;
			pib[3] = base+1;
			pib[4] = base+2;
			pib[5] = base+3;
			pib += 6;
		}
		ib.getObject()->Unlock();
	}
};

/**
 *  Index buffer filler for rectangular grid.
 *
 *  Each cell of a grid is composed of 2 triangles this way:
 *		0----13
 *		|   / |
 *		| /   |
 *		25----4
 *  Vertices are assumed to be laid out in rows.
 */
class CIBFillerGrid : public IIndexBufferFiller {
public:
	CIBFillerGrid( int cols, int rows ) : mCols(cols), mRows(rows) { }

	virtual void fillIB( CD3DIndexBuffer& ib, int elements, D3DFORMAT format ) {
		assert( format == D3DFMT_INDEX16 );
		assert( elements == getElementCount() );
		unsigned short* pib = NULL;
		ib.getObject()->Lock( 0, elements*2, (void**)&pib, 0 );
		assert( &ib );
		for( int i = 0; i < mRows-1; ++i ) {
			for( int j = 0; j < mCols-1; ++j ) {
				unsigned short base = i * mCols + j;
				pib[0] = base;
				pib[1] = base+1;
				pib[2] = base+mCols;
				pib[3] = base+1;
				pib[4] = base+mCols+1;
				pib[5] = base+mCols;
				pib += 6;
			}
		}
		ib.getObject()->Unlock();
	};

	int getElementCount() { return (mCols-1)*(mRows-1)*6; }

private:
	int		mCols;
	int		mRows;
};

}; // namespace

#endif
