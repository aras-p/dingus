// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "TextureHelper.h"

using namespace dingus;


CTextureHelper::CTextureHelper( int framesPerX, int framesPerY, bool addEmptyFrames )
{
	assert( framesPerX > 0 );
	assert( framesPerY > 0 );

	mFrameCount = framesPerX * framesPerY;

	if( addEmptyFrames ) {
		mMin = new SVector2[mFrameCount + 2];
		mMax = new SVector2[mFrameCount + 2];
		mMin[0] = mMax[0] = mMin[mFrameCount+2-1] = mMax[mFrameCount+2-1] = SVector2(0,0);
	} else {
		mMin = new SVector2[mFrameCount];
		mMax = new SVector2[mFrameCount];
	}

	float sizex = 1.0f / framesPerX;
	float sizey = 1.0f / framesPerY;
	for( int i = 0; i < mFrameCount; ++i ) {
		int x = i % framesPerX;
		int y = i / framesPerX;

		int k = addEmptyFrames ? 1 : 0;

		mMin[i+k] = SVector2(  x    * sizex,  y    * sizey );
		mMax[i+k] = SVector2( (x+1) * sizex, (y+1) * sizey );
	}

	if( addEmptyFrames )
		mFrameCount += 2;
}
