// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_CHUNK_HELPER_H
#define __VB_CHUNK_HELPER_H

#include "BufferChunk.h"
#include "ChunkSource.h"


namespace dingus {



/**
 *  Helper class for managing more complex chunk locking scenarios.
 *
 *  Enables locking whole needed portion in smaller chunks, and then
 *  rendering everything. Depending of situation, this can be faster
 *  than locking whole big portion.
 */
class CVBChunkHelper {
	typedef TVBChunk::TSharedPtr		SHARED_CHUNK;
	typedef std::vector<SHARED_CHUNK>	TChunkVector;

public:
	CVBChunkHelper( IChunkSource<TVBChunk>& source )
		: mSource(&source), mCurrentChunkLocked(false) { }

	/**
	 *  Lock some element count. Put the obtained chunk in the end of list for
	 *  later rendering. If there was a previous locked chunk, it is unlocked.
	 *
	 *  @param count Element (eg. vertex or index) count.
	 */
	byte* lock( int count );
	
	/**
	 *  Unlock the last locked chunk.
	 *  @param count TBD - what is the precise meaning?
	 */
	void unlock( int count = 0 ) {
		ASSERT_MSG( mCurrentChunkLocked, "Lock chunk first!" );
		assert( getChunk().get() );
		getChunk()->unlock( count );
		mCurrentChunkLocked = false;
	}

	/**
	 *  Clears locked chunks.
	 */
	virtual void clear() {
		ASSERT_MSG( !mCurrentChunkLocked, "Unlock chunk first!" );
		mChunks.clear();
	}

	/*
	virtual bool render( IVertexStream& stream, bool discardable = true ) {
		assert( mCurrentChunkLocked || "Unlock chunk first!" );

		for( TChunkVector::const_iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk )
		{
			assert( chunk->get() );
			if( !(*chunk)->isValid() )
				return false;
		
			(*chunk)->render( stream, discardable );
		}
		return true;
	}
	*/

	SHARED_CHUNK getChunk() const { return mChunks.back(); }
	SHARED_CHUNK getChunk() { return mChunks.back(); }

private:
	// disable copy
	CVBChunkHelper( CVBChunkHelper const& source );
	CVBChunkHelper& operator= ( CVBChunkHelper const& rhs );
	
private:
	IChunkSource<TVBChunk>*		mSource;
	TChunkVector	mChunks;
	bool			mCurrentChunkLocked;
};


}; // namespace


#endif
