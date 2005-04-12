// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "UIRenderer.h"
#include "UIObject.h"

using namespace dingus;


// --------------------------------------------------------------------------

CGUIObjectsRenderer::CGUIObjectsRenderer( CD3DTexture& defaultTexture, IGUITextRenderer& guiTextRenderer, CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName )
:	mBillboarder(ib,texParamName),
	mDefaultTexture(&defaultTexture),
	mGuiTextRenderer(&guiTextRenderer)
{
	setCoordsScale( 1.0f,1.0f );
	setUVScale( 256.0f,256.0f );
}

void CGUIObjectsRenderer::setCoordsScale( float x, float y )
{
	mCoordsScaleX = x;
	mCoordsScaleY = y;
	mTwoDivCoordsScaleX = 2.0f / x;
	mTwoDivCoordsScaleY = 2.0f / y;
}

void CGUIObjectsRenderer::setUVScale( float x, float y )
{
	mUVScaleX = x;
	mUVScaleY = y;
	mOneDivUVScaleX = 1.0f / x;
	mOneDivUVScaleY = 1.0f / y;
}

void CGUIObjectsRenderer::discardBillboards()
{
	mBillboarder.clear();
}

void CGUIObjectsRenderer::render( CGUIObject& mainObject ) 
{
	mainObject.render( *this );
}

void CGUIObjectsRenderer::renderText( const std::string& text, CGUIFont& font,
		float x1, float y1, float x2, float y2,
		eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV, D3DCOLOR color )
{
	mGuiTextRenderer->renderText( text, font, x1,y1,x2,y2, fit, alignH, alignV, color );
}

void CGUIObjectsRenderer::renderImage( float x1, float y1, float x2, float y2,
		float uvx1, float uvy1, float uvx2, float uvy2,
		D3DCOLOR color, CD3DTexture* texture, bool scaleUV )
{
	assert( mDefaultTexture && mDefaultTexture->getObject() );
	assert( !texture || texture->getObject() );
	const float fixX = mTwoDivCoordsScaleX, fixY = mTwoDivCoordsScaleY;
	float uvX, uvY;
	if( scaleUV ) {
		uvX = mOneDivUVScaleX; uvY = mOneDivUVScaleY;
	} else {
		uvX = uvY = 1.0f;
	}

	SOBillboard& b = mBillboarder.addBill();
	b.x1 = x1 * fixX - 1.0f;	b.y1 = y1 * fixY - 1.0f;
	b.x2 = x2 * fixX - 1.0f;	b.y2 = y2 * fixY - 1.0f;
	b.tu1 = uvx1 * uvX;		b.tv1 = uvy1 * uvY;
	b.tu2 = uvx2 * uvX;		b.tv2 = uvy2 * uvY;
	b.color = color;
	b.texture = texture ? texture : mDefaultTexture;
}

