// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_CHUNK_LOCK_H
#define __VB_CHUNK_LOCK_H

#include "VBChunkHelper.h"


namespace dingus {

	
/**
 *  VB chunk lock object.
 *
 *  Locks the chunk in constructor, unlocks in destructor.
 */
class CVBChunkLock {
public:
	CVBChunkLock( CVBChunkHelper& vbChunk, int vertexCount )
		: mChunkHelper(&vbChunk), mRawData(NULL), mVertexCount(vertexCount), mStride(0), mTouchedVertices(0)
	{
		mRawData = vbChunk.lock( vertexCount );
		assert( vbChunk.getChunk().get() );
		mStride = vbChunk.getChunk()->getStride();
	}
	
	~CVBChunkLock()
	{
		// TBD: possible exception in destructor
		mChunkHelper.unlock( mTouchedVertices );
	}

	void setTouchedRegion( int touchedVertices ) { 
		assert( touchedVertices >= 0 && touchedVertices <= mVertexCount );
		mTouchedVertices = touchedVertices;
	}
	
	byte* getRawData() const { return mRawData; }
	int getVertexCount() const { return mVertexCount; }
	int getStride() const { return mStride; }
	int getTouchedRegion() const { return mTouchedVertices; }

private:
	// disable copy
	CVBChunkLock( CVBChunkLock const& rhs );
	CVBChunkLock const& operator= ( CVBChunkLock const& rhs );

private:
	CVBChunkHelper*	mChunkHelper;
	
	byte*	mRawData;
	int		mVertexCount;
	int		mStride;
	
	int		mTouchedVertices;
};



template <class VERTEX>
class CVBBaseIterator {
public:
	CVBBaseIterator( CVBChunkHelper& vbChunk, int vertexCount ) : mLockedChunk( vbChunk, vertexCount ) { }
	
	void setTouchedRegion( int touchedVertices ) { mLockedChunk.setTouchedRegion( touchedVertices ); }
	int getTouchedRegion() const { return mLockedChunk.getTouchedRegion(); }

protected:
	CVBChunkLock mLockedChunk;
};


template <class VERTEX>
class CVBChunkRawIterator : public CVBBaseIterator<VERTEX> {
public:
	CVBChunkRawIterator( CVBChunkHelper& vbChunk, int vertexCount ) 
		: CVBBaseIterator<VERTEX>( vbChunk, vertexCount )
	{
		assert( sizeof( VERTEX ) == mLockedChunk.getStride() );
		mData = reinterpret_cast<VERTEX*>( mLockedChunk.getRawData() );
	}

	operator VERTEX* const& () const { return mData; }
	operator VERTEX*& () { return mData; }

private:	
	// disable copy
	CVBChunkRawIterator( CVBChunkRawIterator const& source );
	CVBChunkRawIterator& operator= ( CVBChunkRawIterator const& rhs );

private:
	VERTEX*			mData;
};


}; // namespace


#endif
