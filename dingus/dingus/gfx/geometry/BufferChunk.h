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
	CBufferChunk( BUFFER& buffer, unsigned char* data, unsigned int offset, unsigned int count, unsigned int stride );
	
	void unlock( unsigned int count = 0 );
	unsigned char* getData() const { return mData; }
	unsigned int getOffset() const { return mOffset; }
	unsigned int getSize() const { return mCount; }
	bool isValid() const { return mValid; }
	unsigned int getStride() const { return mStride; }
	BUFFER& getBuffer() const { assert(mBuffer); return *mBuffer; }
	
	void invalidate() { mValid = false; }

protected:
	BUFFER*		mBuffer;	// The buffer
	unsigned char*	mData;		// Pointer to start of chunk data
	unsigned int	mOffset;	// Offset from buffer start, in elements (not bytes!)
	unsigned int	mCount;		// Element (not bytes!) count in chunk
	unsigned int	mStride;	// Element size in bytes

	bool		mValid;		// Is this chunk still valid?
};



template< typename BUFFER >
CBufferChunk<BUFFER>::CBufferChunk( BUFFER& buffer, unsigned char* data, unsigned int offset, unsigned int count, unsigned int stride )
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
void CBufferChunk<BUFFER>::unlock( unsigned int count )
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
