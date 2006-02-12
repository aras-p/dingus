// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SAMPLED_ANIMATION_H
#define __SAMPLED_ANIMATION_H

#include "AnimCurve.h"
#include "Animation.h"
#include "../math/TypeTraits.h"


namespace dingus {


/**
 *  A sampled animation.
 *
 *  Each curve consists of the same number of samples (but can be collapsed
 *  into single value).
 */
template<typename _V>
class CSampledAnimation : public IAnimation<_V> {
public:
	/**
	 *  Animation loop type.
	 *  CLAMP clamps animation to ends.
	 *  REPEAT loops animation (with interpolation from last sample to first).
	 *  REPEAT_LAST loops animation (no interpolation from last to first,
	 *   instead, the last sample is for transition from pre-last to last).
	 */
	enum eLoopType { CLAMP = 0, REPEAT, REPEAT_LAST };
	typedef CAnimCurve<value_type>	curve_type;

public:
	CSampledAnimation( int samplesInCurve, eLoopType loopType ) : mSamplesInCurve(samplesInCurve), mLoopType(loopType) { };

	void	addSample( const value_type& sample ) { mSamples.push_back(sample); }
	void	reserveSamples( int sampleCount ) { mSamples.reserve(sampleCount); }
	void	resizeSamples( int sampleCount ) { mSamples.resize(sampleCount); }
	int		getTotalSampleCount() const { return (int)mSamples.size(); }
	const value_type& getSample( int index ) const;

	void	addCurve( const curve_type& curve ) { mCurves.push_back(curve); }
	void	reserveCurves( int curveCount ) { mCurves.reserve(curveCount); }
	int		getCurveCount() const { return (int)mCurves.size(); }
	const curve_type& getCurve( int index ) const;

	int		getSamplesInCurve() const { return mSamplesInCurve; }

	void		setLoopType( eLoopType loopType ) { mLoopType = loopType; }
	eLoopType	getLoopType() const { return mLoopType; }

	/**
	 *  @param time Relative time (zero is start, one is end).
	 */
	void	timeToIndex( float time, int& index0, int& index1, int& index2, int& index3, float& alpha ) const;
	
	// IAnimation
	virtual void sample( float time, int firstCurve, int numCurves, value_type* dest, int destStride = sizeof(value_type) ) const;
	virtual float getLength() const { return float(mSamplesInCurve); };

private:
	typedef std::vector<value_type>	TSampleVector;
	typedef std::vector<curve_type> TCurveVector;

private:
	TSampleVector	mSamples;
	TCurveVector	mCurves;
	int				mSamplesInCurve;
	eLoopType		mLoopType;
};


// --------------------------------------------------------------------------

template<typename _V>
inline void CSampledAnimation<_V>::timeToIndex( float time, int& index0, int& index1, int& index2, int& index3, float& alpha ) const
{
	int n = mSamplesInCurve;
	float frame = time * (mLoopType==REPEAT_LAST ? n-1 : n);
	int iframe = int(frame);
	index0 = iframe-1;
	index1 = iframe;
	index2 = iframe + 1;
	index3 = iframe + 2;
	alpha = frame - float(iframe);
	
	switch( mLoopType ) {
	case CLAMP:
		if( index0 < 0 ) index0 = 0;
		else if( index0 >= n ) index0 = n-1;
		if( index1 < 0 ) index1 = 0;
		else if( index1 >= n ) index1 = n-1;
		if( index2 < 0 ) index2 = 0;
		else if( index2 >= n ) index2 = n-1;
		if( index3 < 0 ) index3 = 0;
		else if( index3 >= n ) index3 = n-1;
		break;
	case REPEAT:
		index0 %= n;
		index1 %= n;
		index2 %= n;
		index3 %= n;
		break;
	case REPEAT_LAST:
		index0 %= (n-1);
		index1 %= (n-1);
		index2 = index1 + 1;
		index3 = index1 + 2;
		if( index3 >= n ) index3 = n-1;
	}
};

template<typename _V>
inline const _V& CSampledAnimation<_V>::getSample( int index ) const
{
	assert( index>=0 && index < getTotalSampleCount() );
	return mSamples[index];
};

template<typename _V>
inline const CAnimCurve<_V>& CSampledAnimation<_V>::getCurve( int index ) const
{
	assert( index>=0 && index < getCurveCount() );
	return mCurves[index];
};

template<typename _V>
void CSampledAnimation<_V>::sample( float time, int firstCurve, int numCurves, _V* dest, int destStride ) const
{
	assert( firstCurve >= 0 && firstCurve < getCurveCount() );
	assert( numCurves > 0 && numCurves <= getCurveCount() );
	assert( firstCurve+numCurves <= getCurveCount() );
	assert( dest );
	assert( destStride >= sizeof(_V) );

	// get sample indices and alpha
	int smpidx0, smpidx1, smpidx2, smpidx3;
	float alpha;
	timeToIndex( time, smpidx0, smpidx1, smpidx2, smpidx3, alpha );

	for( int i = 0; i < numCurves; ++i ) {
		const curve_type& curve = getCurve( firstCurve + i );
		curve_type::eIpol ipol = curve.getIpol();
		int fstsmp = curve.getFirstSampleIndex();

		switch( ipol ) {
		case curve_type::NONE:
			*dest = curve.getCollapsedValue();
			break;
		case curve_type::STEP:
			*dest = getSample( fstsmp + smpidx1 );
			break;
		case curve_type::LINEAR:
			{
			const value_type& s1 = getSample( fstsmp + smpidx1 );
			const value_type& s2 = getSample( fstsmp + smpidx2 );
			*dest = math_type_traits<value_type>::interpolate( s1, s2, alpha );
			}
			break;
			/*
		case curve_type::LINEAR:
			{
			const value_type& s0 = getSample( fstsmp + smpidx0 );
			const value_type& s1 = getSample( fstsmp + smpidx1 );
			const value_type& s2 = getSample( fstsmp + smpidx2 );
			const value_type& s3 = getSample( fstsmp + smpidx3 );
			*dest = math_type_traits<value_type>::cm_interpolate( s0, s1, s2, s3, alpha );
			}
			break;
			*/
		};

		((const char*&)dest) += destStride;
	};
};


}; // namespace

#endif
