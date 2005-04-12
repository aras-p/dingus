// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __INPUT_FILTER_H
#define __INPUT_FILTER_H

#include "../math/TypeTraits.h"

namespace dingus {


// --------------------------------------------------------------------------

/**
 *  Filters input values over several frames.
 *  @param T Input value type.
 *		Must have (T += T) and (T *= float) operators and math_type_traits<T>::identify().
 */
template< typename T >
class CInputFilter {
public:
	/// Constructor.
	CInputFilter( int historySize = 4 ) { setSize(historySize); }

	/// Set history size.
	void setSize( int sz );

	/// Push new value
	void pushValue( const T& v );

	/// Get filtered value.
	T getFilteredValue() const;

	/// Get a raw unfiltered value.
	const T& getRawValue( int index ) const;
	
private:
	enum { MAX_HISTORY_SIZE = 8 };
	int		mSize;
	float	mInvSize;
	T		mValues[MAX_HISTORY_SIZE];
};


// --------------------------------------------------------------------------

template< typename T >
inline void CInputFilter<T>::setSize( int hs )
{
	assert( hs > 1 );
	if( hs > MAX_HISTORY_SIZE )
		hs = MAX_HISTORY_SIZE;
	mSize = hs;
	mInvSize = 1.0f / float(mSize);
	
	for( int i = 0; i < mSize; ++i )
		math_type_traits<T>::identify( mValues[i] );
}

template< typename T >
inline void CInputFilter<T>::pushValue( const T& val )
{
	// move existing entries
	for( int i = mSize-1; i > 0; --i )
		mValues[i] = mValues[i-1];
	// write new value to beginning
	mValues[0] = val;
}

template< typename T >
inline T CInputFilter<T>::getFilteredValue() const
{
	T val = math_type_traits<T>::identity();
	for( int i = 0; i < mSize; ++i )
		val += mValues[i];
	val *= mInvSize;
	return val;
}

template< typename T >
inline const T& CInputFilter<T>::getRawValue( int index ) const
{
	assert( index >= 0 && index < mSize );
	return mValues[index];
}


}; // namespace

#endif
