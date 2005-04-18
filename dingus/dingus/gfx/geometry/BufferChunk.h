// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __BUFFER_CHUNK_H
#define __BUFFER_CHUNK_H

#include "../../kernel/Proxies.h"

namespace dingus {

// --------------------------------------------------------------------------

/// Class for some buffer chunk. Represents locked portion of the buffer.
template< typename BUFFER >
class CBufferChunk : public CRefCounted {
public:
	typedef typename DingusSmartPtr< CBufferChunk<BUFFER> >	TSharedPtr;
	
public:
	CBufferChunk( BUFFER& buffer, unsigned char* data, int offset, int count, int stride );
	
	void unlock( int count = 0 );
	unsigned char* getData() const { return mData; }
	int getOffset() const { return mOffset; }
	int getSize() const { return mCount; }
	bool isValid() const { return mValid; }
	int getStride() const { return mStride; }
	BUFFER& getBuffer() const { assert(mBuffer); return *mBuffer; }
	
	void invalidate() { mValid = false; }

protected:
	BUFFER*		mBuffer;	// The buffer
	unsigned char*		mData;		// Pointer to start of chunk data
	int			mOffset;	// Offset from buffer start, in elements (not bytes!)
	int			mCount;		// Element (not bytes!) count in chunk
	int			mStride;	// Element size in bytes

	bool		mValid;		// Is this chunk still valid?
};



template< typename BUFFER >
CBufferChunk<BUFFER>::CBufferChunk( BUFFER& buffer, unsigned char* data, int offset, int count, int stride )
:	mBuffer( &buffer ),
	mData( data ),
	mOffset( offset ),
	mCount( count ),
	mStride( stride ),
	mValid( true )
{
	assert( mBuffer );
}


template< typename BUFFER >
void CBufferChunk<BUFFER>::unlock( int count )
{
	if( mCount )
		mBuffer->getObject()->Unlock();
	mData = NULL;
	mValid = true;
	if( !count )
		return;
	assert( count > 0 );
	mCount = count;
}


// --------------------------------------------------------------------------


typedef CBufferChunk<CD3DVertexBuffer>	TVBChunk;
typedef CBufferChunk<CD3DIndexBuffer>	TIBChunk;


}; // namespace


#endif
