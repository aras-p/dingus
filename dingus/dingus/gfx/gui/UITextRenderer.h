// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_TEXT_RENDERER_H
#define __UI_TEXT_RENDERER_H

#include "UIObjectRenderer.h"

namespace dingus {


class CGUITextRenderer : public IGUITextRenderer {
public:
	CGUITextRenderer() : mGuiImageRenderer(0) { };
	void setGuiImageRenderer( IGUIImageRenderer& renderer ) { mGuiImageRenderer = &renderer; }

	// IGUITextRenderer
	virtual void renderText( const std::string& text, CGUIFont& font,
		float x1, float y1, float x2, float y2,
		eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV, D3DCOLOR color );

	float calcHeight( const std::string& text, const CGUIFont& font, float fitWidth ) const;
	float calcWidth( const std::string& text, const CGUIFont& font, float fitHeigth ) const;
	float calcNativeWidth( const std::string& text, const CGUIFont& font ) const;

private:
	IGUIImageRenderer* mGuiImageRenderer;
};

}; // namespace

#endif
