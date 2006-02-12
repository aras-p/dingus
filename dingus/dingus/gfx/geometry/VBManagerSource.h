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
	CVBManagerSource( unsigned int stride ) : mStride(stride) { }
	virtual SHARED_CHUNK lock( unsigned int count ) { return CDynamicVBManager::getInstance().allocateChunk( count, mStride ); }
private:
	unsigned int	mStride;
};

}; // namespace

#endif
