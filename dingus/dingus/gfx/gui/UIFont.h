// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_FONT_H
#define __UI_FONT_H

#include "../../kernel/Proxies.h"

namespace dingus {


class CGUIFont : public boost::noncopyable {
public:
	struct SLetter {
	public:
		float	u0, v0, u1, v1;
		float	width, height; // in texels. always integers, preconverted to floats for speed
	};
public:
	CGUIFont( int letterCount, int firstLetter, float maxWidth, float maxHeight, CD3DTexture& texture );
	~CGUIFont();

	float	getMaxLetterWidth() const { return mMaxLetterWidth; }
	float	getMaxLetterHeight() const { return mMaxLetterHeight; }

	CD3DTexture& getTexture() { return *mTexture; }

	/**
	 *  @return Letter struct. For non-existing letters, special letter is returned.
	 */
	const SLetter& getLetter( int ascii ) const;

	// Use for loading font info only!
	SLetter& getLetterByNumber( int i ) { return mLetters[i]; }

private:
	int				mLetterCount;
	int				mFirstLetter; // ascii code
	float			mMaxLetterWidth; // in texels
	float			mMaxLetterHeight; // in texels
	SLetter*		mLetters; // last one - bogus for error letters
	CD3DTexture*	mTexture;
};

}; // namespace

#endif
