// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "MemoryPool.h"

using namespace dingus;


CMemoryPool::CMemoryPool( int blockSize, int numElements, bool dontGrow, const char* name )
:	mBlockSize( blockSize < sizeof(void*) ? sizeof(void*) : blockSize ),
	mBlocksPerBubble( numElements ),
	mDontGrow( dontGrow ),
	mPeakAllocCount(0),
	mName(name)
{
	reset();
	allocNewBubble();
}

CMemoryPool::~CMemoryPool()
{
	if( mAllocCount > 0 ) {
		ASSERT_FAIL_MSG( "leaked!" );
	}
	deallocateAll();
}

void CMemoryPool::reset()
{
	mAllocCount = 0;
	mHeadOfFreeList = 0;
}

void CMemoryPool::deallocateAll()
{
	stl_utils::wipe( mBubbles );
	reset();
}

void CMemoryPool::allocNewBubble()
{
	int sizeMultiplier;
	
	if( mDontGrow && !mBubbles.empty() ) {
		ASSERT_FAIL_MSG( "CMemoryPool::allocNewBubble: can't grow" );
		return;
	}
	
	sizeMultiplier = mBubbles.size() + 1;
	
	int elemCount = mBlocksPerBubble * sizeMultiplier;
	int bubbleSize = mBlockSize * elemCount;
	SBubble *bubble = (SBubble*)malloc( sizeof(SBubble) + bubbleSize - 1 );
	assert( bubble );
	
	bubble->mByteCount = bubbleSize;

	// put to bubble list
	mBubbles.push_back( bubble );
	
	// setup the free list inside a bubble
	mHeadOfFreeList = bubble->mData;
	assert( mHeadOfFreeList );
	
	void **newBubble = (void**)mHeadOfFreeList;
	for( int j = 0; j < elemCount-1; ++j ) {
		newBubble[0] = (char*)newBubble + mBlockSize;
		newBubble = (void**)newBubble[0];
	}
	newBubble[0] = NULL; // null terminate the list
}

void* CMemoryPool::allocate()
{
	return allocate( mBlockSize );
}

void *CMemoryPool::allocate( unsigned int amount )
{
	void *returnBlock;
	
	if( amount > (unsigned int)mBlockSize ) {
		ASSERT_FAIL_MSG( "requested larger amount than block size!" );
		return NULL;
	}
	
	if( !mHeadOfFreeList ) {
		// returning NULL is fine in no-grow mode
		if( mDontGrow )
			return NULL;
		
		// allocate new bubble
		allocNewBubble();
		
		// still failure, error out
		if( !mHeadOfFreeList ) {
			ASSERT_FAIL_MSG( "CMemoryPool::allocate: out of memory" );
			return NULL;
		}
	}

	++mAllocCount;
	if( mAllocCount > mPeakAllocCount )
		mPeakAllocCount = mAllocCount;
	
	returnBlock = mHeadOfFreeList;
	
	// move the pointer the next block
	mHeadOfFreeList = *((void**)mHeadOfFreeList);
	
	return returnBlock;
}

void CMemoryPool::deallocate( void *memBlock )
{
	if( !memBlock ) // ignore NULL deletes
		return;
	
#ifdef _DEBUG
	// check to see if the memory is from the allocated range
	bool ok = false;
	int n = mBubbles.size();
	for( int i = 0; i < n; ++i ) {
		SBubble* p = mBubbles[i];
		if( (char*)memBlock >= p->mData && (char*)memBlock < (p->mData + p->mByteCount) ) {
			ok = true;
			break;
		}
	}
	assert( ok );
#endif // _DEBUG
	
#ifdef _DEBUG	
	// invalidate the memory
	memset( memBlock, 0xDD, mBlockSize );
#endif
	
	--mAllocCount;
	
	// make the block point to the first free item in the list
	*((void**)memBlock) = mHeadOfFreeList;
	// the list head is now the deallocated block
	mHeadOfFreeList = memBlock;
}
