// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_RENDERER_H
#define __UI_RENDERER_H

#include "../RenderableOrderedBillboards.h"
#include "UIObjectRenderer.h"

namespace dingus {

class CGUIObject;
class CGUIObjectHolder;
class CGUIImageObject;


// --------------------------------------------------------------------------

class CGUIObjectsRenderer : public IGUIObjectRenderer {
public:
	CGUIObjectsRenderer( CD3DTexture& defaultTexture, IGUITextRenderer& guiTextRenderer, CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName = "tBase" );
	~CGUIObjectsRenderer() { discardBillboards(); }

	/**
	 *  Set coordinates scale.
	 *  All coordinates are from (0,0) at top-left of rendertarget,
	 *  to 'scale' at bottom right.
	 */
	void setCoordsScale( float x, float y );

	/**
	 *  Set UV coordinates scale.
	 *  All coordinates are from (0,0) at top-left, to 'scale' at bottom right.
	 */
	void setUVScale( float x, float y );

	void render( CGUIObject& mainObject );
	void discardBillboards();

	CRenderableOrderedBillboards& getBillboarder() { return mBillboarder; }

	// IUIObjectRenderer
	virtual void renderText( const std::string& text, CGUIFont& font,
		float x1, float y1, float x2, float y2,
		eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV, D3DCOLOR color );
	virtual void renderImage( float x1, float y1, float x2, float y2,
		float uvx1, float uvy1, float uvx2, float uvy2,
		D3DCOLOR color, CD3DTexture* texture, bool scaleUV );

private:
	CRenderableOrderedBillboards	mBillboarder;
	CD3DTexture*			mDefaultTexture;
	IGUITextRenderer*		mGuiTextRenderer;
	float	mCoordsScaleX, mCoordsScaleY;
	float	mTwoDivCoordsScaleX, mTwoDivCoordsScaleY;
	float	mUVScaleX, mUVScaleY;
	float	mOneDivUVScaleX, mOneDivUVScaleY;
};


}; // namespace

#endif
