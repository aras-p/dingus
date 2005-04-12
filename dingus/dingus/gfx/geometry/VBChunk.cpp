// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VBChunk.h"

using namespace dingus;



CVBChunk::CVBChunk( CD3DVertexBuffer& vb, unsigned char* data, int offset, int count, int stride )
:	mVB( &vb ),
	mData( data ),
	mOffset( offset ),
	mCount( count ),
	mStride( stride ),
	mValid( true )
{
}

void CVBChunk::unlock( int count )
{
	if( mCount )
		mVB->getObject()->Unlock();
	mData = NULL;
	mValid = true;
	if( !count )
		return;
	assert( count > 0 );
	mCount = count;
}

