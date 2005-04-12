// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATH_UTILS_H
#define __MATH_UTILS_H

#include "FPU.h"


namespace dingus {


/**
 *  Clamps value to the given range.
 */
inline float clamp( float value, float vmin = 0.0f, float vmax = 1.0f ) {
	assert( vmin <= vmax );
	if( value < vmin )
		value = vmin;
	else if( value > vmax )
		value = vmax;
	return value;
};


/**
 *  Smoothing function.
 *
 *  Uses critically damped spring for ease-in/ease-out smoothing. Stable
 *  at any time intervals. Based on GPG4 article.
 *
 *  @param from	Current value.
 *  @param to	Target value (may be moving).
 *	@param vel	Velocity (updated by the function, should be maintained between calls).
 *	@param smoothTime	Time in which the target should be reached, if travelling at max. speed.
 *	@param dt	Delta time.
 *	@return Updated value.
 */
template< typename T >
inline T smoothCD( const T& from, const T& to, T& vel, float smoothTime, float dt )
{
	float omega = 2.0f / smoothTime;
	float x = omega * dt;
	// approximate exp()
	float exp = 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x );
	T change = from - to;
	T temp = ( vel + omega * change ) * dt;
	vel = ( vel - omega * temp ) * exp;
	return to + ( change + temp ) * exp;
}



template< typename T >
class CTabularFunction {
public:
	CTabularFunction( int nvals, const T* vals )
		: mValueCount(nvals)
		, mValueCountMinusOne(nvals-1)
	{
		assert( vals );
		assert( nvals > 0 );
		mValues = new T[ nvals ];
		memcpy( mValues, vals, nvals*sizeof(T) );
	}
	~CTabularFunction()
	{
		delete[] mValues;
	}

	void	eval( float t, T& val ) const {
		float alpha = clamp( t, 0.0f, 1.0f ) * mValueCountMinusOne;
		int idx0 = round( alpha - 0.5f );
		assert( idx0 >= 0 && idx0 < mValueCount );
		int idx1 = idx0 + 1;
		if( idx1 >= mValueCount )
			idx1 = mValueCount-1;
		float lerp = alpha - idx0;
		val = mValues[idx0] + (mValues[idx1]-mValues[idx0]) * lerp;
	}

private:
	int		mValueCount;
	T*		mValues;
	float	mValueCountMinusOne;
};



}; // namespace

#endif
