// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef D3D_FONT_H__
#define D3D_FONT_H__

namespace dingus {

/**
 *  Texture-based font class for doing text in a 3D scene.
 *  Note: messes up with device state, so reset cached state/objs after using it.
 */
class CD3DFont {
public:
	enum eCreateFlags { BOLD = 1, ITALIC = 2, ZENABLE = 4 };
	enum eRenderFlags { CENTERED_X = 1, CENTERED_Y = 2, TWO_SIDED = 4, FILTERED = 8 };

public:
    CD3DFont( const TCHAR* fontName, int height, int flags=0 );
    ~CD3DFont();

    HRESULT drawText(
		float x, float y, D3DCOLOR color, const TCHAR* str, int flags=0 );

	/*
    HRESULT drawTextScaled(
		float x, float y, float z, float xScale, float yScale,
		D3DCOLOR color, const TCHAR* str, int flags=0 );
    
	HRESULT render3DText( const TCHAR* str, int flags=0 );
    
    HRESULT getTextExtent( const TCHAR* str, SIZE& resSize ) const;
	*/

    // Initializing and destroying device-dependent objects
    HRESULT createDeviceObjects();
    HRESULT activateDeviceObjects();
    HRESULT passivateDeviceObjects();
    HRESULT deleteDeviceObjects();


protected:
    TCHAR   mFontName[80];
    int		mFontHeight;
    int		mFontFlags;

	IDirect3DTexture9*		mTexture;
	IDirect3DVertexBuffer9*	mVB;
	int		mTexWidth;
	int		mTexHeight;
	float	mTextScale;
	float	mTexCoords[128-32][4];
	int		mSpacing;

	IDirect3DStateBlock9*	mBlockSaved;
	IDirect3DStateBlock9*	mBlockDrawText;
};

}; // namespace dingus


#endif


