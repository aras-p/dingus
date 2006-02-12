// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

namespace dingus {
	

// --------------------------------------------------------------------------

class CMemoryPool {
public:
	CMemoryPool( int blockSize, int numElements, bool dontGrow, const char* name = 0 );
	~CMemoryPool();
	
	/// Allocate single block
	void*	allocate();
	/// Allocate less than single block
	void*	allocate( size_t amount );
	/// Deallocate
	void	deallocate( void *ptr );
	/// Deallocate everyting
	void	deallocateAll();
	
private:
	struct SBubble {
		size_t 	mByteCount;
		char	mData[1]; // actually mByteCount
	};
	typedef std::vector<SBubble*>	TBubbleVector;
	
	void	reset();
	void	allocNewBubble();
	void	reportLeaks();
	
private:
	int 		mBlockSize;
	int 		mBlocksPerBubble;
	
	bool		mDontGrow;
	
	TBubbleVector	mBubbles;

	void*	mHeadOfFreeList;
	int 	mAllocCount; // number of blocks currently allocated
	int 	mPeakAllocCount; // stats

	const char*	mName; // for debugging mainly
};


// --------------------------------------------------------------------------
//  Macros for class fixed-size pooled allocators:
//		DECLARE_POOLED_ALLOC in the private section of a class,
//		DEFINE_POOLED_ALLOC in the .cpp file

#ifdef NO_MMGR


#define DECLARE_POOLED_ALLOC( _clazz ) \
public:	\
	inline void* operator new( size_t size ) { return mPoolAllocator.allocate(size); } \
	/*inline void* operator new( size_t size, int nBlockUse, const char *pFileName, int nLine ) { return mPoolAllocator.allocate(size); } */ \
	inline void	operator delete( void* p ) { mPoolAllocator.deallocate(p); } \
	/*inline void	operator delete( void* p, int nBlockUse, const char *pFileName, int nLine ) { mPoolAllocator.deallocate(p); } */ \
private: \
	static dingus::CMemoryPool mPoolAllocator
	
#define DEFINE_POOLED_ALLOC( _clazz, _bubbleSize, _noGrow ) \
	dingus::CMemoryPool _clazz::mPoolAllocator(sizeof(_clazz), _bubbleSize, _noGrow, #_clazz)


#else


#define DECLARE_POOLED_ALLOC( _clazz )
#define DEFINE_POOLED_ALLOC( _clazz, _bubbleSize, _noGrow )


#endif



}; // namespace

#endif
