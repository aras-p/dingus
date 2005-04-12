// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"

#include "../../input/InputEvent.h"
#include "UIObjects.h"
#include "UIObjectRenderer.h"

using namespace dingus;


// --------------------------------------------------------------------------
//  CGUIImage

CGUIImage::CGUIImage(
	float x1, float y1, float x2, float y2,
	float uvx1, float uvy1, float uvx2, float uvy2,
	D3DCOLOR color, CD3DTexture* tex )
:	CGUIObject(x1,y1,x2,y2),
	mUVX1(uvx1), mUVY1(uvy1), mUVX2(uvx2), mUVY2(uvy2),
	mColor(color), mTexture(tex)
{
}

void CGUIImage::render( IGUIObjectRenderer& renderer )
{
	if( !isVisible() )
		return;
	renderer.renderImage(
		getX1(),getY1(),getX2(),getY2(),
		mUVX1, mUVY1, mUVX2, mUVY2,
		mColor, mTexture, true );
}




// --------------------------------------------------------------------------
//  CGUILabel

CGUILabel::CGUILabel(
	float x1, float y1, float x2, float y2,
	const std::string& text, CGUIFont& font,
	eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV,
	D3DCOLOR color )
:	CGUIObject(x1,y1,x2,y2),
	mText(text), mFont(&font),
	mFit(fit), mAlignH(alignH), mAlignV(alignV),
	mColor( color )
{
}

void CGUILabel::render( IGUIObjectRenderer& renderer )
{
	if( !isVisible() || mText.size() == 0 )
		return;
	renderer.renderText(
		mText, *mFont,
		getX1(),getY1(),getX2(),getY2(),
		mFit, mAlignH, mAlignV, mColor );
}


// --------------------------------------------------------------------------
//  CGUIInputBox

CGUIInputBox::CGUIInputBox( float x1, float y1, float x2, float y2,
	const std::string& text, CGUIFont& font,
	eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV,
	D3DCOLOR color, int inputType, int maxLength )
:	CGUILabel( x1, y1, x2, y2, text, font, fit, alignH, alignV, color ),
	mInputType( inputType ), mHasFocus( true ), mBlinkTime( 0 ),
	mMaxLength( maxLength ), mShifOn( false )
{
}

void CGUIInputBox::render( IGUIObjectRenderer& renderer )
{
	mBlinkTime += CSystemTimer::getInstance().getDeltaTimeS();

	if( mBlinkTime > 0.5f ) {
		mBlinkTime = 0;
	
		if( getText().size() > mText.size() ) setText( mText );
		else setText( mText + "|" );
	} 		

	CGUILabel::render( renderer );	
}

void CGUIInputBox::onInputEvent( const CInputEvent& e )
{
	if( e.getType() == CKeyEvent::EVENT_TYPE ) {
		const CKeyEvent& ke = (const CKeyEvent&)e;

		if( ( mInputType | CAPS_LETTERS ) &&
			( ke.getKeyCode() == DIK_RSHIFT || ke.getKeyCode() == DIK_LSHIFT ) ) 
		{
			if( ke.getMode() == CKeyEvent::KEY_DOWN ) mShifOn = true;
			else if( ke.getMode() == CKeyEvent::KEY_RELEASED ) mShifOn = false;
		}

		if( ke.getMode() != CKeyEvent::KEY_RELEASED ) return;

		char c = ke.getAscii();
		if( c > 0 && mText.size() < mMaxLength ) {
			if( ( ( mInputType | LETTERS ) && c >= 'A' && c <= 'Z' ) ||
				( ( mInputType | SPACE ) && c == ' ' ) )
			{
				if( c >= 'A' && c <= 'Z' && !mShifOn ) mText += ( c - 'A' + 'a' );
				else mText += c;
				
				setText( mText + "|" );
			}
		} 

		if( ke.getKeyCode() == DIK_BACKSPACE && mText.size() > 0 ) {
			mText = mText.substr( 0, mText.size() - 1 );
			setText( mText + "|" );
		}
	}
}