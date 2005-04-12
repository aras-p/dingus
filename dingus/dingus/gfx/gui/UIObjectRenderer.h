// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __UI_OBJECTS_RENDERER_H
#define __UI_OBJECTS_RENDERER_H

#include "../../kernel/Proxies.h"
#include "UIFont.h"

namespace dingus {


enum eUITextAlign {
	UI_A_LEFT = 0, UI_A_TOP = 0,
	UI_A_CENTER = 1,
	UI_A_RIGHT = 2, UI_A_BOTTOM = 2
};
enum eUITextFit {
	UI_FIT_HEIGHT, UI_FIT_WIDTH, UI_FIT_RECT
};

class IGUITextRenderer {
public:
	virtual void renderText( const std::string& text, CGUIFont& font,
		float x1, float y1, float x2, float y2,
		eUITextFit fit, eUITextAlign alignH, eUITextAlign alignV, D3DCOLOR color ) = 0;
};

class IGUIImageRenderer {
public:
	/**
	 *  Render image at given global coordinates, UV portion, color and
	 *  optional texture (if no texture is given, the renderer uses "default"
	 *  one).
	 */
	virtual void renderImage( float x1, float y1, float x2, float y2,
		float uvx1, float uvy1, float uvx2, float uvy2,
		D3DCOLOR color, CD3DTexture* texture, bool scaleUV ) = 0;
};

class IGUIObjectRenderer : public IGUITextRenderer, public IGUIImageRenderer {
};

}; // namespace

#endif