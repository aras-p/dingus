// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MANAGED_BUFFER_H
#define __MANAGED_BUFFER_H

#include "ChunkSource.h"
#include "../../console/Console.h"

namespace dingus {

	
template <typename CHUNK, typename BUFFER>
class CManagedBuffer {
	typedef typename CHUNK::TSharedPtr		TChunkPtr;
	typedef	typename fastvector<TChunkPtr>	TChunkVector;
	
public:
	CManagedBuffer( unsigned int capacityBytes )
	:	mBuffer(NULL), mCapacityBytes(capacityBytes), mUsedBytes(0)
	{
		// NOTE: generally don't allocate buffer here.
	}
	
	virtual ~CManagedBuffer()
	{
		// force discard
		discard();
	}
	
	TChunkPtr allocateChunk( unsigned int count, unsigned int stride )
	{
		if( count == 0 )
			return TChunkPtr();

		assert( mBuffer );

		unsigned int byteCount = count*stride;
		ASSERT_MSG( byteCount <= mCapacityBytes, "too much requested for lock. increase capacity of buffer" );
		if( byteCount > mCapacityBytes ) {
			CConsoleChannel& ch = CConsole::CON_WARNING;
			ch << "DynaBuffer: too much requested for lock" << endl;
			ch << "req bytes=" << byteCount << ", capacity bytes=" << mCapacityBytes << endl;
			ch << "req elems=" << count << ", capacity elems=" << mCapacityBytes/stride << endl;
			return TChunkPtr();
		}
		unsigned int modulo = mUsedBytes%stride;
		unsigned int byteStart = mUsedBytes + (modulo ? (stride-modulo) : 0);
		if( byteStart + byteCount > mCapacityBytes ) {
			CConsoleChannel& ch = CConsole::CON_WARNING;
			ch << "DynaBuffer: filled, discarding" << endl;
			ch << "req bytes=" << byteCount << ", capacity bytes=" << mCapacityBytes << endl;
			ch << "req elems=" << count << ", capacity elems=" << mCapacityBytes/stride << endl;
			discard();
			byteStart = 0;
		}
		
		byte* data = lockBuffer( byteStart, byteCount );
		
		TChunkPtr chunk = TChunkPtr( new CHUNK( *mBuffer, data, byteStart/stride, count, stride ) );
		assert( chunk.get() );

		mChunks.push_back( chunk );
		
		mUsedBytes = byteStart + byteCount;
		return chunk;
	}
	
	void discard()
	{
		TChunkVector::iterator it, itEnd = mChunks.end();
		for( it = mChunks.begin(); it != itEnd; ++it ) {
			assert( it->get() );
			(*it)->invalidate();
		}
		mChunks.clear();
		mUsedBytes = 0;
	}

	BUFFER*		getBuffer() { return mBuffer; }

protected:
	virtual byte* lockBuffer( unsigned int byteStart, unsigned int byteCount ) = 0;
	unsigned int getCapacityBytes() { return mCapacityBytes; }

protected:
	BUFFER*		mBuffer;

private:
	unsigned int	mCapacityBytes;
	unsigned int	mUsedBytes;
	
	TChunkVector mChunks;
};


}; // namespace

#endif
