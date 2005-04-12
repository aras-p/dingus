// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __POOL_BY_VALUE_H__
#define __POOL_BY_VALUE_H__

#include "fastvector.h"

namespace dingus {

//------------------------------------------------------------------------

/**
 *  Pool holds arbitrary type items by-value.
 *
 *  It's good for the cases where allocation of type is costly, as pool
 *  re-uses previously freed instances when allocation is needed. Adding to
 *  pool in normal case takes constant time. Non-normal cases arise when pool
 *  must grow in size.
 *
 *  The limitation is that pool objects must be equal in size. Type must have copy
 *  constructor and (not sure) default constructor.
 */

template<class T>
class CPool {
public:
	class iterator;
	class const_iterator;

private:

	/*
	 *  A tiny small pool (hence a pool-let).
	 *  As items in pool are kept by-value, the pool cannot reallocate them if there's
	 *  not enough space. So instead it has a list of constant-sized pool-lets; and
	 *  upon depletion of empty space, allocates whole new pool-let. Pool-let is
	 *  deallocated when it becomes empty.
	 */

	//
	// pool-let
	class CPoollet {
	public:
		typedef std::vector<T>		TDataVector;
		typedef std::vector<int>	TIndexVector;
		friend class iterator;
		friend class const_iterator;
		friend class CPool<T>;

	public:
		CPoollet( int space ) : mSize(0) {
			assert( space >= 1 );
			mData.reserve( space );
			mIndices.reserve( space );
			for( int i = 0; i < space; i++ ) {
				mData.push_back( T() );
				mIndices.push_back( i );
			}
		}

		int size() const { return mSize; };

		T& add( const T& value ) {
			assert( hasSpace() );
			int slot = mIndices[ mSize ];	// get empty slot index
			mData[slot] = value;			// fill it
			mSize++;						// grow
			return mData[slot];				// return the real object
		}
		
		/** Has this poollet some empty space left? */
		bool hasSpace() const { return (size_t)mSize < mData.size(); };

	private:
		TDataVector		mData;
		TIndexVector	mIndices;
		int				mSize;
	};

private:
	typedef fastvector<CPoollet*>	TPoolletVector;
	int				mPoolletSize;
	TPoolletVector	mPoollets;
	int				mSize;

public:
	//
	// const iterator
	friend class const_iterator;
	class const_iterator {
		friend class iterator;
	public:
		const_iterator()
			{}
		const_iterator( const typename TPoolletVector::const_iterator& plt, int idx = 0 )
			: mPoollet(plt), mIndex( idx ) { }
		const_iterator( const const_iterator& ci )
			: mPoollet(ci.mPoollet), mIndex(ci.mIndex) { }
		const_iterator( const iterator& ci )
			: mPoollet(ci.mPoollet), mIndex(ci.mIndex) { }
		const T& operator*() const {
			return (*mPoollet)->mData[ (*mPoollet)->mIndices[ mIndex ] ];
		}
		const T* operator->() const {
			return &**this;
		}
		const_iterator& operator++() {
			mIndex++;
			if( mIndex >= (*mPoollet)->mSize ) {
				mPoollet++;
				mIndex = 0;
			}
			return *this;
		}
		const_iterator operator++(int) {
			const_iterator ti = *this;
			++*this;
			return ti;
		}
		bool operator==( const const_iterator& ci ) const {
			return mPoollet == ci.mPoollet && mIndex == ci.mIndex;
		}
		bool operator!=( const const_iterator& ci ) const {
			return !(*this == ci);
		}

	protected:
		TPoolletVector::const_iterator	mPoollet;
		int								mIndex;
	};

	//
	// iterator
	friend class iterator;
	class iterator {
		friend class const_iterator;
	public:
		iterator()
			{}
		iterator( const TPoolletVector::iterator& plt, int idx = 0 )
			: mPoollet(plt), mIndex( idx ) { }
		iterator( const iterator& ci )
			: mPoollet(ci.mPoollet), mIndex(ci.mIndex) { }
		iterator( const const_iterator& ci )
			: mPoollet(ci.mPoollet), mIndex(ci.mIndex) { }
		T& operator*() const {
			return (*mPoollet)->mData[ (*mPoollet)->mIndices[ mIndex ] ];
		}
		T* operator->() const {
			return &**this;
		}
		iterator& operator++() {
			mIndex++;
			if( mIndex >= (*mPoollet)->mSize ) {
				mPoollet++;
				mIndex = 0;
			}
			return *this;
		}
		iterator operator++(int) {
			iterator ti = *this;
			++*this;
			return ti;
		}
		bool operator==( const iterator& ci ) const {
			return mPoollet == ci.mPoollet && mIndex == ci.mIndex;
		}
		bool operator!=( const iterator& ci ) const {
			return !(*this == ci);
		}

		TPoolletVector::iterator&	getPoollet() { return mPoollet; }
		const TPoolletVector::iterator& getPoollet() const { return mPoollet; }
		int& getIndex() { return mIndex; }
		const int& getIndex() const { return mIndex; }

	protected:
		TPoolletVector::iterator	mPoollet;
		int							mIndex;
	};


	iterator begin() { return iterator( mPoollets.begin() ); }
	const_iterator begin() const { return const_iterator( mPoollets.begin() ); }
	iterator end() { return iterator( mPoollets.end() ); }
	const_iterator end() const { return const_iterator( mPoollets.end() ); }

public:
	explicit CPool( int poolletSize = 128 );
	~CPool();
	
	int size() const { return mSize; }
	bool empty() const { return mSize == 0; }

	void clear();

	iterator erase( iterator it );

	T& add( const T& value = T() );
};

//------------------------------------------------------------------------

template<class T>
CPool<T>::CPool( int poolletSize )
:	mPoolletSize(poolletSize),
	mPoollets(),
	mSize(0)
{
}

template<class T>
CPool<T>::~CPool()
{
	clear();
}

template<class T>
void CPool<T>::clear()
{
	TPoolletVector::iterator pli;
	for( pli = mPoollets.begin(); pli != mPoollets.end(); ++pli )
		delete *pli;
	mPoollets.clear();
	mSize = 0;
}

template<class T>
T& CPool<T>::add( const T& value = T() )
{
	// search for space in poollets
	TPoolletVector::iterator pli;
	for( pli = mPoollets.begin(); pli != mPoollets.end(); ++pli ) {
		CPoollet& pl = **pli;
		if( pl.hasSpace() ) { // found space
			mSize++;
			return pl.add( value );
		}
	}
	// no space. allocate new poollet
	CPoollet *pl = new CPoollet( mPoolletSize );
	mPoollets.push_back( pl );
	mSize++;
	return pl->add( value );
}

template<class T>
CPool<T>::iterator CPool<T>::erase( iterator it )
{
	TPoolletVector::iterator plti = it.getPoollet();
	CPoollet& plt = **plti;
	int idx = it.getIndex();
	// remove and place last in place of it
	int i1 = idx;
	int i2 = plt.mSize-1;
	// swap indices
	int t = plt.mIndices[ i1 ];
	plt.mIndices[ i1 ] = plt.mIndices[ i2 ];
	plt.mIndices[ i2 ] = t;
	--plt.mSize;
	--mSize;
	// remove poollet if empty
	if( plt.mSize == 0 ) {
		delete &plt;
		plti = mPoollets.erase( plti );
		it = iterator( plti, 0 );
	} else if( idx >= plt.mSize ) {
		it = iterator( ++plti, 0 );
	}
	return it;
}


}; // namespace

#endif

