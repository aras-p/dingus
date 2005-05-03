// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __IMMEDIATE_MIXER_H_
#define __IMMEDIATE_MIXER_H_

#include "AnimStream.h"

namespace dingus {


/**
 *  Animation stream immediate mixer.
 *
 *  Weight-blends any number of animation streams upon explicit command.
 *
 *  @param _S Stream type (specialization of CAnimStream).
 */
template<typename _S>
class CAnimImmediateMixer {
public:
	typedef _S							stream_type;
	typedef typename _S::TSharedPtr		streamptr_type;
	typedef typename _S::value_type		value_type;
	typedef CAnimImmediateMixer<_S>		this_type;
	
	/** Stream data: the stream and it's weight. */
	struct SStreamData {
		SStreamData( const streamptr_type& stream, float weight, int skipCurves, int useCurves )
			: mStream(stream), mWeight(weight), mSkipCurves(skipCurves), mUseCurves(useCurves) { }
		streamptr_type	mStream;
		float			mWeight;
		int				mSkipCurves; // if 0 - use all
		int				mUseCurves; // if 0 - use all
	};

public:
	// ----------------------------------------------------------------------

	/// Adds a stream with weight.
	void addStream( const streamptr_type& stream, float weight, int skipCurves = 0, int useCurves = 0 ) {
		mStreams.push_back( SStreamData( stream, weight, skipCurves, useCurves ) );
	}
	/// Clears the streams.
	void clearStreams() { mStreams.clear(); }

	/// Blend the streams into provided destination array.
	void update( time_value timenow, int numCurves, value_type* dest, bool normalizeWeights, int destStride = sizeof(value_type) ) {
		int i, n = mStreams.size();

		// no streams?
		if( !n ) return;

		// place for scratch values
		if( mScratchValues.size() < numCurves )
			mScratchValues.resize( numCurves );

		// possibly normalize weights
		if( normalizeWeights ) {
			float wsum = 0.0f;
			for( i = 0; i < n; ++i )
				wsum += mStreams[i].mWeight;
			float invWSum = 1.0f / wsum;
			for( i = 0; i < n; ++i )
				mStreams[i].mWeight *= invWSum;
		}

		// go thru streams and weight-blend them into dest
		bool firstStream = true;
		float accumWeight = 0.0f; // for quat blending
		for( i = 0; i < n; ++i ) {
			const SStreamData& s = mStreams[i];
			float streamWeight = s.mWeight;
			// just skip zero-weight streams
			if( streamWeight <= 0.0f )
				continue;
			// sample stream into scratch
			s.mStream->update( timenow, &mScratchValues[0], sizeof(value_type) );
			// blend all or a subset of curves
			char* destVal = reinterpret_cast<char*>(dest);
			int ncrv = numCurves;
			if( s.mUseCurves > 0 ) {
				assert( s.mSkipCurves >= 0 );
				assert( s.mUseCurves > 0 );
				assert( s.mSkipCurves + s.mUseCurves <= numCurves );
				ncrv = s.mSkipCurves + s.mUseCurves;
				destVal += destStride * s.mSkipCurves;
			}
			for( int c = s.mSkipCurves; c < ncrv; ++c, destVal += destStride ) {
				value_type& dv = *reinterpret_cast<value_type*>(destVal);
				if( firstStream )
					SBlender<value_type>::calcFirst( dv, mScratchValues[c], streamWeight );
				else 
					SBlender<value_type>::calcBlend( dv, mScratchValues[c], streamWeight, accumWeight );
			}
			firstStream = false;
			accumWeight += streamWeight;
		}
	};

private:

	// ----------------------------------------------------------------------
	// Private stuff

	typedef std::vector<SStreamData> TStreamVector;
	typedef std::vector<value_type>	 TValueVector;

	// Blend specialization code.
	template<typename _T> struct SBlender {
		static void calcFirst( _T& dest, const _T& val, float weight ) {
			dest = val * weight;
		};
		static void calcBlend( _T& dest, const _T& val, float weight, float accumWeight ) {
			dest += val * weight;
		};
	};
	// Blend specialization code for quaternions.
	template<> struct SBlender<SQuaternion> {
		static void calcFirst( SQuaternion& dest, const SQuaternion& val, float weight ) {
			dest = val;
		};
		static void calcBlend( SQuaternion& dest, const SQuaternion& val, float weight, float accumWeight ) {
			float w = weight / ( accumWeight + weight );
			dest.slerp( dest, val, w );
		};
	};

private:
	TStreamVector	mStreams;
	TValueVector	mScratchValues;
};

}; // namespace

#endif
