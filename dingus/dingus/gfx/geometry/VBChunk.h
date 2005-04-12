// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_CHUNK_H
#define __VB_CHUNK_H

#include "../../kernel/Proxies.h"

namespace dingus {

/**
 *  Vertex buffer chunk. Represents some locked portion of vertex buffer.
 */
class CVBChunk : public CRefCounted {
public:
	typedef DingusSmartPtr<CVBChunk>	TSharedPtr;

public:
	CVBChunk( CD3DVertexBuffer& vb, unsigned char* data, int offset, int count, int stride );
	
	void unlock( int count = 0 );
	unsigned char* getData() const { return mData; }
	int getOffset() const { return mOffset; }
	int getSize() const { return mCount; }
	bool isValid() const { return mValid; }
	int getStride() const { return mStride; }
	CD3DVertexBuffer& getVB() const { return *mVB; }
	
	void invalidate() { mValid = false; }

private:
	CD3DVertexBuffer*	mVB;	// Vertex buffer
	unsigned char*		mData;		// Pointer to start of chunk data
	int			mOffset;	// Offset from buffer start, in elements (not bytes!)
	int			mCount;		// Element (not butes!) count in chunk
	int			mStride;	// Element size in bytes

	bool		mValid;		// Is this chunk still valid?
};


}; // namespace


#endif
