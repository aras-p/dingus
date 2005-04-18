// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_CHUNK_SOURCE_H
#define __VB_CHUNK_SOURCE_H

#include "BufferChunk.h"
#include "DynamicVBManager.h"


namespace dingus {

/**
 *  Chunk source that provides VB chunks from CDynamicVBManager.
 */
class CVBManagerSource : public IChunkSource<TVBChunk> {
public:
	CVBManagerSource( int stride ) : mStride(stride) { }
	virtual SHARED_CHUNK lock( int count ) { return CDynamicVBManager::getInstance().allocateChunk( count, mStride ); }
private:
	int					mStride;
};

}; // namespace

#endif
