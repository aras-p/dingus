// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIMATION_STREAM_H
#define __ANIMATION_STREAM_H

#include "../utils/Timer.h"
#include "Animation.h"
#include "../math/Vector3.h"
#include "../math/Quaternion.h"


namespace dingus {


// --------------------------------------------------------------------------

/**
 *  Animation stream.
 *
 *  Has the underlying animation, knows starting curve and curve count.
 *  Manages time.
 *
 *  Is ref counted.
 *
 *  @param _V Value type.
 */
template<typename _V>
class CAnimStream : public CRefCounted {
public:
	typedef _V value_type;
	typedef CAnimStream<_V> this_type;
	typedef DingusSmartPtr<this_type> TSharedPtr;
	typedef IAnimation<value_type> animation_type;
	
public:
	CAnimStream( const animation_type& animation, float duration, int firstCurveIdx, int numCurves, time_value startTime )
		: mAnimation( &animation )
		, mDuration(duration)
		, mOneOverDuration(1.0f/duration)
		, mStartTime(startTime)
		, mFirstCurveIndex( firstCurveIdx )
		, mNumCurves( numCurves )
	{
	}

	virtual ~CAnimStream() { }


	float	getDuration() const { return mDuration; }
	time_value	getStartTime() const { return mStartTime; }
	float	getRelTime( time_value timenow ) const { return (timenow-mStartTime).tosec() * mOneOverDuration; }
	float	getOneOverDuration() const { return mOneOverDuration; }

	int		getFirstCurveIndex() const { return mFirstCurveIndex; }

	/// Abruptly sets duration. Most likely you don't want this.
	void	setDuration( float d ) { mDuration = d; mOneOverDuration = 1.0f / d; }

	/// Alters duration in such way so that animation keeps playing smoothly from current position.
	void	adjustDuration( float d, time_value timenow ) {
		if( mDuration == d )
			return;
		float relt = getRelTime( timenow );
		setDuration( d );
		mStartTime = timenow - time_value::fromsec( relt * d );
	}


	/**
	 *  Update animation stream, write current values into user provided space.
	 *  @param dest Start of destination space.
	 *  @param destStride Stride between adjacent values in bytes.
	 */
	void update( time_value timenow, value_type* dest, int destStride = sizeof(value_type) ) const
	{
		assert( mAnimation );
		float t = getRelTime( timenow );
		mAnimation->sample( t, mFirstCurveIndex, mNumCurves, dest, destStride );
	}

private:
	const animation_type*	mAnimation;
	time_value	mStartTime;
	float	mDuration;
	float	mOneOverDuration;
	int		mFirstCurveIndex;
	int		mNumCurves;
};


// --------------------------------------------------------------------------

typedef CAnimStream<SVector3>		TVectorAnimStream;
typedef CAnimStream<SQuaternion>	TQuatAnimStream;
typedef CAnimStream<float>			TFloatAnimStream;


}; // namespace


#endif
