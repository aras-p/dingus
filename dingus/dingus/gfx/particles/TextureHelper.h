// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TEXTURE_HELPER_H
#define __TEXTURE_HELPER_H

#include "../../math/Vector2.h"


namespace dingus {



class CTextureHelper {
public:
	/**
	 *  @param addEmptyFrames If this is set, the total number of frames is
	 *  increased by 2 - it's perX*perY+2 in total. The first and the last frames
	 *  are dummy - they refer to zero size texture region at zero UV.
	 */
	CTextureHelper( int framesPerX, int framesPerY, bool addEmptyFrames );
	~CTextureHelper() { delete[] mMin; delete[] mMax; }

	const SVector2& getMin( int frame ) const { assert( frame>=0 && frame<mFrameCount ); return mMin[frame]; }
	const SVector2& getMax( int frame ) const { assert( frame>=0 && frame<mFrameCount ); return mMax[frame]; }

private:
	int			mFrameCount;
	SVector2*	mMin;
	SVector2*	mMax;
};


}; // namespace


#endif