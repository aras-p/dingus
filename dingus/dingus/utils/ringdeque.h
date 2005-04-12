// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RING_DEQUE_H
#define __RING_DEQUE_H

#include <cassert>


namespace dingus {


/**
 *  Fixed capacity deque with zero (re)allocations.
 *
 *  Basically, you can push/pop back/front of it; the data is stored in
 *  a ring buffer style array.
 *
 *  Type T must have default constructor. Removed entries are
 *  assigned with default T instance.
 *
 *  NOTE: not fully STL container compatible (e.g. no iterators yet, no STL like datatypes).
 */
template< typename T, int CAPACITY >
class ringdeque {
public:
	typedef typename ringdeque<T,CAPACITY> this_type;

public:
	explicit ringdeque()
		: mFirstIdx(0)
		, mSize(0)
	{
	}
	~ringdeque()
	{
	}

	//this_type& operator=( const this_type& other ) { v = other.v; return *this; }
	
	int		capacity() const { return CAPACITY; }
	int		size() const { return mSize; }
	bool	empty() const { return mSize == 0; }
	bool	full() const { return mSize == CAPACITY; }

	const T& operator[]( int pos ) const {
		return mData[ index2data( pos ) ];
	}
	T& operator[]( int pos ) {
		return mData[ index2data( pos ) ];
	}

	const T& back() const {
		assert( !empty() );
		return mData[ index2data( mSize-1 ) ];
	}
	T& back() {
		assert( !empty() );
		return mData[ index2data( mSize-1 ) ];
	}
	const T& front() const {
		assert( !empty() );
		return mData[ mFirstIdx ];
	}
	T& front() {
		assert( !empty() );
		return mData[ mFirstIdx ];
	}


	void	push_back( const T& val ) {
		assert( !full() );
		++mSize;
		mData[index2data(mSize-1)] = val;
	}
	void	pop_back() {
		assert( !empty() );
		mData[index2data(mSize-1)] = T();
		--mSize;
	}

	void	push_front( const T& val ) {
		assert( !full() );
		mFirstIdx = (mFirstIdx-1+CAPACITY) % CAPACITY;
		mData[mFirstIdx] = val;
		++mSize;
	}
	void	pop_front() {
		assert( !empty() );
		mData[mFirstIdx] = T();
		mFirstIdx = (mFirstIdx+1) % CAPACITY;
		--mSize;
	}

	void	clear() {
		int idx = mFirstIdx;
		for( int i = 0; i < mSize; ++i ) {
			mData[idx] = T();
			idx = (idx+1) % CAPACITY;
		}
		mSize = 0;
		mFirstIdx = 0;
	}

private:
	int		index2data( int pos ) const {
		assert( pos >= 0 && pos < mSize );
		return (mFirstIdx + pos) % CAPACITY;
	}

private:
	T		mData[CAPACITY];
	int		mFirstIdx;
	int		mSize;
};

}; // namespace


#endif
