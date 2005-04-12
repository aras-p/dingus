// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "UIFont.h"

using namespace dingus;

CGUIFont::CGUIFont( int letterCount, int firstLetter, float maxWidth, float maxHeight, CD3DTexture& texture )
:	mLetterCount(letterCount), mFirstLetter(firstLetter),
	mMaxLetterWidth(maxWidth), mMaxLetterHeight(maxHeight),
	mTexture(&texture)
{
	assert( letterCount > 0 );
	mLetters = new SLetter[letterCount+1];
	assert( mLetters );
	SLetter& bogus = mLetters[letterCount]; // last one - whole texture
	bogus.u0 = bogus.v0 = 0.0f;
	bogus.u1 = bogus.v1 = 1.0f;
	bogus.width = maxWidth;
	bogus.height = maxHeight;
}

CGUIFont::~CGUIFont()
{
	assert( mLetters );
	delete[] mLetters;
}

const CGUIFont::SLetter& CGUIFont::getLetter( int ascii ) const
{
	if( ascii < mFirstLetter || ascii >= mFirstLetter+mLetterCount )
		return mLetters[mLetterCount];
	else
		return mLetters[ascii-mFirstLetter];
}
