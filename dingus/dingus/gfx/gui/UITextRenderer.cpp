// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "UITextRenderer.h"

using namespace dingus;

float CGUITextRenderer::calcNativeWidth( const std::string& text, const CGUIFont& font ) const
{
	assert( text.size() > 0 );
	float width = 0.0f;
	int n = text.size();
	for( int i = 0; i < n; ++i ) {
		width += font.getLetter( text[i] ).width;
	}
	assert( width > 0.0f );
	return width;
}

float CGUITextRenderer::calcHeight( const std::string& text, const CGUIFont& font, float fitWidth ) const
{
	float width = calcNativeWidth( text, font );
	return font.getMaxLetterHeight() * (fitWidth / width);
}

float CGUITextRenderer::calcWidth( const std::string& text, const CGUIFont& font, float fitHeight ) const
{
	float width = calcNativeWidth( text, font );
	return width * (fitHeight / font.getMaxLetterHeight() );
}

void CGUITextRenderer::renderText( const std::string& text, CGUIFont& font,
		float x1, float y1, float x2, float y2,
		eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV, D3DCOLOR color )
{
	assert( mGuiImageRenderer );

	float nativeWidth = calcNativeWidth( text, font );

	//
	// calc scaling

	float scaleX, scaleY;
	float rectX = x2-x1, rectY = y2-y1;
	switch( fit ) {
	case UI_FIT_HEIGHT:
		scaleX = scaleY = rectY / font.getMaxLetterHeight();
		break;
	case UI_FIT_WIDTH:
		scaleX = scaleY = rectX / nativeWidth;
		break;
	case UI_FIT_RECT:
		scaleX = rectX / nativeWidth;
		scaleY = rectY / font.getMaxLetterHeight();
		break;
	default:
		ASSERT_FAIL_MSG( "bad fit value!" );
	}

	//
	// align

	float textWidth = nativeWidth * scaleX;
	float textHeight = font.getMaxLetterHeight() * scaleY;
	float cornerX = x1, cornerY = y1;
	if( alignH == UI_A_CENTER )
		cornerX += (rectX-textWidth) * 0.5f;
	else if( alignH == UI_A_RIGHT )
		cornerX += (rectX-textWidth);
	if( alignV == UI_A_CENTER )
		cornerY += (rectY-textHeight) * 0.5f;
	else if( alignV == UI_A_BOTTOM )
		cornerY += (rectY-textHeight);

	// 
	// render

	const float startCornerX = cornerX;

	int n = text.size();
	for( int i = 0; i < n; ++i ) {
		if( text[i] == '\n' || text[i] == '\r' ) {
			cornerX = startCornerX;
			cornerY += textHeight;
		} else {
			const CGUIFont::SLetter& l = font.getLetter( text[i] );
			const float csX = l.width*scaleX;
			const float csY = l.height*scaleY;
			mGuiImageRenderer->renderImage( cornerX, cornerY,
				cornerX + csX, cornerY + csY,
				l.u0, l.v0, l.u1, l.v1, color, &font.getTexture(), false );
			cornerX += csX;
		}
	}
}
