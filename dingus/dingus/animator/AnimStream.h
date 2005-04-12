// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIMATION_STREAM_H
#define __ANIMATION_STREAM_H

#include "AnimTime.h"


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
	CAnimStream( const animation_type& animation, float duration, int firstCurveIdx, int numCurves, float startTime = anim_time() )
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
	float	getStartTime() const { return mStartTime; }
	float	getRelTime( float timenow = anim_time() ) const { return (timenow-mStartTime) * mOneOverDuration; }
	float	getOneOverDuration() const { return mOneOverDuration; }

	int		getFirstCurveIndex() const { return mFirstCurveIndex; }

	/// Abruptly sets duration. Most likely you don't want this.
	void	setDuration( float d ) { mDuration = d; mOneOverDuration = 1.0f / d; }

	/// Alters duration in such way so that animation keeps playing smoothly from current position.
	void	adjustDuration( float d ) {
		if( mDuration == d )
			return;
		float t = anim_time();
		float relt = getRelTime( t );
		setDuration( d );
		mStartTime = t - relt * d;
	}


	/**
	 *  Update animation stream, write current values into user provided space.
	 *  @param dest Start of destination space.
	 *  @param destStride Stride between adjacent values in bytes.
	 */
	void update( value_type* dest, int destStride = sizeof(value_type) ) const
	{
		assert( mAnimation );
		float t = getRelTime();
		mAnimation->sample( t, mFirstCurveIndex, mNumCurves, dest, destStride );
	}

private:
	const animation_type*	mAnimation;
	float	mDuration;
	float	mOneOverDuration;
	float	mStartTime;
	int		mFirstCurveIndex;
	int		mNumCurves;
};


// --------------------------------------------------------------------------

typedef CAnimStream<SVector3>		TVectorAnimStream;
typedef CAnimStream<SQuaternion>	TQuatAnimStream;
typedef CAnimStream<float>			TFloatAnimStream;


}; // namespace


#endif
