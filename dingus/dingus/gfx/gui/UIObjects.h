// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_OBJECTS_H
#define __UI_OBJECTS_H

#include "UIObject.h"
#include "UIObjectRenderer.h"
#include "../../kernel/Proxies.h"

namespace dingus {

class CInputEvent;


// --------------------------------------------------------------------------

/**
 *  UI image.
 */
class CGUIImage : public CGUIObject {
public:
	CGUIImage(
		float x1, float y1, float x2, float y2,
		float uvx1, float uvy1, float uvx2, float uvy2,
		D3DCOLOR color = 0xFFFFFFFF, CD3DTexture* tex = NULL );

	void setUV( float uvx1, float uvy1, float uvx2, float uvy2 ) {
		mUVX1 = uvx1; mUVY1 = uvy1; mUVX2 = uvx2; mUVY2 = uvy2;
	}
	void setColor( D3DCOLOR col ) { mColor = col; }
	void setTexture( CD3DTexture* tex ) { mTexture = tex; }

	// CGUIObject
	virtual void render( IGUIObjectRenderer& renderer );

protected:
	D3DCOLOR getColor() const { return mColor; }
	CD3DTexture* getTexture() { return mTexture; }

private:
	float		mUVX1, mUVY1, mUVX2, mUVY2;
	D3DCOLOR	mColor;
	CD3DTexture*	mTexture;
};



// --------------------------------------------------------------------------

/**
 *  UI label.
 */
class CGUILabel : public CGUIObject {
public:
	/**
	 *  Label with arbitrary alignment.
	 */
	CGUILabel(
		float x1, float y1, float x2, float y2,
		const std::string& text, CGUIFont& font,
		eUITextFit fit = UI_FIT_HEIGHT, eUITextAlign alignH = UI_A_LEFT, eUITextAlign alignV = UI_A_TOP,
		D3DCOLOR color = 0xFFFFFFFF );

	void setText( const std::string& text ) { mText = text; }
	const std::string& getText() const { return mText; }
	void setFont( CGUIFont& font ) { mFont = &font; }
	void setFit( eUITextFit fit = UI_FIT_HEIGHT ) { mFit = fit; }
	void setAlignH( eUITextAlign a = UI_A_LEFT ) { mAlignH = a; }
	void setAlignV( eUITextAlign a = UI_A_TOP ) { mAlignV = a; }
	void setColor( D3DCOLOR color ) { mColor = color; }

	// CGUIObject
	virtual void render( IGUIObjectRenderer& renderer );

private:
	std::string		mText;
	CGUIFont*		mFont;
	eUITextFit		mFit;
	eUITextAlign	mAlignH;
	eUITextAlign	mAlignV;
	D3DCOLOR		mColor;
};

/**
 *  UI Input box.
 */
class CGUIInputBox : public CGUILabel {
public:
	enum eType { 
		LETTERS			= 1 << 0, 
		CAPS_LETTERS	= 1 << 1, 
		SPACE			= 1 << 2,
		ALL				= 0xff
	};

	/**
	 * inputType should be result of eType params (for example LETTERS | SPACE)
	 */
	CGUIInputBox( float x1, float y1, float x2, float y2,
		const std::string& text, CGUIFont& font,
		eUITextFit fit = UI_FIT_HEIGHT, eUITextAlign alignH = UI_A_LEFT, eUITextAlign alignV = UI_A_TOP,
		D3DCOLOR color = 0xFFFFFFFF, int inputType = ALL, int maxLength = 10 );

	// CGUIObject
	virtual void render( IGUIObjectRenderer& renderer );

	void setHasFocus( bool b ) { mHasFocus = b; }
	void onInputEvent( const CInputEvent& e );

	const std::string getInput() const { return mText; }

private:
	int mInputType;
	int mMaxLength;
	bool mHasFocus;
	std::string mText;
	float mBlinkTime;
	bool mShifOn;
};


}; // namespace

#endif
