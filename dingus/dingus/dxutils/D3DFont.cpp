// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "D3DFont.h"
#include "DXUtil.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


//---------------------------------------------------------------------------
// Custom vertex types for rendering text

#define MAX_NUM_VERTICES 50*6

struct SFont2DVertex { D3DXVECTOR4 p; D3DCOLOR color; 	float tu, tv; };
struct SFont3DVertex { D3DXVECTOR3 p; D3DXVECTOR3 n;	float tu, tv; };

#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_FONT3DVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

inline SFont2DVertex gInitFont2DVertex( const D3DXVECTOR4& p, D3DCOLOR color, float tu, float tv )
{
	SFont2DVertex v;
	v.p = p;	 v.color = color;	v.tu = tu;	 v.tv = tv;
	return v;
}

inline SFont3DVertex gInitFont3DVertex( const D3DXVECTOR3& p, const D3DXVECTOR3& n, float tu, float tv )
{
	SFont3DVertex v;
	v.p = p;	 v.n = n;	v.tu = tu;	 v.tv = tv;
	return v;
}



//---------------------------------------------------------------------------
// CD3DFont()
//---------------------------------------------------------------------------


CD3DFont::CD3DFont( const TCHAR* fontName, int height, int flags )
:	mFontHeight(height), mFontFlags(flags),
	mSpacing(0),
	/*mDevice(NULL), */mTexture(NULL), mVB(NULL),
	mBlockSaved(NULL), mBlockDrawText(NULL)
{
	_tcsncpy( mFontName, fontName, sizeof(mFontName) / sizeof(TCHAR) );
	mFontName[sizeof(mFontName) / sizeof(TCHAR) - 1] = _T('\0');
}


CD3DFont::~CD3DFont()
{
	passivateDeviceObjects();
	deleteDeviceObjects();
}


HRESULT CD3DFont::createDeviceObjects(/* IDirect3DDevice9& device */)
{
	HRESULT hr;
	
	CD3DDevice& device = CD3DDevice::getInstance();
	// Keep a local copy of the device
	//mDevice = &device;
	
	// Establish the font and texture size
	mTextScale	= 1.0f; // Draw fonts into texture without scaling
	
	// Large fonts need larger textures
	if( mFontHeight > 60 )
		mTexWidth = mTexHeight = 2048;
	else if( mFontHeight > 30 )
		mTexWidth = mTexHeight = 1024;
	else if( mFontHeight > 15 )
		mTexWidth = mTexHeight = 512;
	else
		mTexWidth  = mTexHeight = 256;
	
	// If requested texture is too big, use a smaller texture and smaller font,
	// and scale up when rendering.

	int maxTexWidth = device.getCaps().getCaps().MaxTextureWidth;
	if( mTexWidth > maxTexWidth ) {
		mTextScale = (float)maxTexWidth / (float)mTexWidth;
		mTexWidth = mTexHeight = maxTexWidth;
	}
	
	// Create a new texture for the font
	hr = device.getDevice().CreateTexture(
		mTexWidth, mTexHeight, 1, 0, D3DFMT_A4R4G4B4,
		D3DPOOL_MANAGED, &mTexture, NULL );
	if( FAILED(hr) )
		return hr;
	
	// Prepare to create a bitmap
	DWORD*		pBitmapBits;
	BITMAPINFO	bmi;
	ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		=  (int)mTexWidth;
	bmi.bmiHeader.biHeight		= -(int)mTexHeight;
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount	= 32;
	
	// Create a DC and a bitmap for the font
	HDC 	hDC 	  = CreateCompatibleDC( NULL );
	HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0 );
	SetMapMode( hDC, MM_TEXT );
	
	// Create a font.  By specifying ANTIALIASED_QUALITY, we might get an
	// antialiased font, but this is not guaranteed.
	INT nHeight    = -MulDiv( mFontHeight, (INT)(GetDeviceCaps(hDC, LOGPIXELSY) * mTextScale), 72 );
	DWORD dwBold   = (mFontFlags&BOLD)	 ? FW_BOLD : FW_NORMAL;
	DWORD dwItalic = (mFontFlags&ITALIC) ? TRUE    : FALSE;
	HFONT hFont    = CreateFont(
		nHeight, 0, 0, 0, dwBold, dwItalic,
		FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		VARIABLE_PITCH, mFontName );
	if( NULL==hFont )
		return E_FAIL;
	
	HGDIOBJ hbmOld = SelectObject( hDC, hbmBitmap );
	HGDIOBJ hFontOld = SelectObject( hDC, hFont );
	
	// Set text properties
	SetTextColor( hDC, RGB(255,255,255) );
	SetBkColor(   hDC, 0x00000000 );
	SetTextAlign( hDC, TA_TOP );
	
	// Loop through all printable character and output them to the bitmap..
	// Meanwhile, keep track of the corresponding tex coords for each character.
	int x = 0;
	int y = 0;
	TCHAR str[2] = _T("x");
	SIZE size;
	
	// Calculate the spacing between characters based on line height
	GetTextExtentPoint32( hDC, TEXT(" "), 1, &size );
	x = mSpacing = (int)ceil(size.cy * 0.3f);
	
	for( TCHAR c=32; c<127; c++ ) {
		str[0] = c;
		GetTextExtentPoint32( hDC, str, 1, &size );
		
		if( x + size.cx + mSpacing > mTexWidth ) {
			x  = mSpacing;
			y += size.cy+1;
		}
		
		ExtTextOut( hDC, x+0, y+0, ETO_OPAQUE, NULL, str, 1, NULL );
		
		mTexCoords[c-32][0] = ((float)(x + 0       - mSpacing))/mTexWidth;
		mTexCoords[c-32][1] = ((float)(y + 0       + 0       ))/mTexHeight;
		mTexCoords[c-32][2] = ((float)(x + size.cx + mSpacing))/mTexWidth;
		mTexCoords[c-32][3] = ((float)(y + size.cy + 0       ))/mTexHeight;
		
		x += size.cx + (2 * mSpacing);
	}
	
	// Lock the surface and write the alpha values for the set pixels
	D3DLOCKED_RECT d3dlr;
	mTexture->LockRect( 0, &d3dlr, 0, 0 );
	BYTE* pDstRow = (BYTE*)d3dlr.pBits;
	WORD* pDst16;
	BYTE bAlpha; // 4-bit measure of pixel intensity
	
	for( y=0; y < mTexHeight; y++ ) {
		pDst16 = (WORD*)pDstRow;
		for( x=0; x < mTexWidth; x++ ) {
			bAlpha = (BYTE)((pBitmapBits[mTexWidth*y + x] & 0xff) >> 4);
			if (bAlpha > 0) {
				*pDst16++ = (WORD) ((bAlpha << 12) | 0x0fff);
			} else {
				*pDst16++ = 0x0000;
			}
		}
		pDstRow += d3dlr.Pitch;
	}
	
	// Done updating texture, so clean up used objects
	mTexture->UnlockRect(0);
	SelectObject( hDC, hbmOld );
	SelectObject( hDC, hFontOld );
	DeleteObject( hbmBitmap );
	DeleteObject( hFont );
	DeleteDC( hDC );
	
	return S_OK;
}



HRESULT CD3DFont::activateDeviceObjects()
{
	HRESULT hr;
	
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();

	// Create vertex buffer for the letters
	int vertexSize = sizeof(SFont2DVertex) > sizeof(SFont3DVertex ) ? sizeof(SFont2DVertex) : sizeof(SFont3DVertex);
	if( FAILED( hr = dx.CreateVertexBuffer(
		MAX_NUM_VERTICES * vertexSize,
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0,
		D3DPOOL_DEFAULT, &mVB, NULL ) ) )
	{
		return hr;
	}
	
	// Create the state blocks for rendering text
	for( UINT which=0; which<2; which++ ) {
		dx.BeginStateBlock();
		dx.SetTexture( 0, mTexture );
		
		if( ZENABLE & mFontFlags )
			dx.SetRenderState( D3DRS_ZENABLE, TRUE );
		else
			dx.SetRenderState( D3DRS_ZENABLE, FALSE );
		
		dx.SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		dx.SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
		dx.SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
		dx.SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		dx.SetRenderState( D3DRS_ALPHAREF,		 0x08 );
		dx.SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
		dx.SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
		dx.SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
		dx.SetRenderState( D3DRS_STENCILENABLE,	 FALSE );
		dx.SetRenderState( D3DRS_CLIPPING,		 TRUE );
		dx.SetRenderState( D3DRS_CLIPPLANEENABLE,  FALSE );
		dx.SetRenderState( D3DRS_VERTEXBLEND, 	 D3DVBF_DISABLE );
		dx.SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		dx.SetRenderState( D3DRS_FOGENABLE,		 FALSE );
		dx.SetRenderState( D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED  | D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );
		dx.SetTextureStageState( 0, D3DTSS_COLOROP,	D3DTOP_MODULATE );
		dx.SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dx.SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dx.SetTextureStageState( 0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE );
		dx.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dx.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		dx.SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		dx.SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		dx.SetTextureStageState( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
		dx.SetTextureStageState( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );
		dx.SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		dx.SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		dx.SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
		
		if( which==0 )
			dx.EndStateBlock( &mBlockSaved );
		else
			dx.EndStateBlock( &mBlockDrawText );
	}
	
	return S_OK;
}


HRESULT CD3DFont::passivateDeviceObjects()
{
	safeRelease( mVB );
	safeRelease( mBlockSaved );
	safeRelease( mBlockDrawText );
	
	return S_OK;
}


HRESULT CD3DFont::deleteDeviceObjects()
{
	safeRelease( mTexture );
	//mDevice = NULL;
	
	return S_OK;
}



/*
HRESULT CD3DFont::getTextExtent( const TCHAR* str, SIZE& resSize ) const
{
	if( !str )
		return E_FAIL;
	
	float fRowWidth  = 0.0f;
	float fRowHeight = (mTexCoords[0][3]-mTexCoords[0][1])*mTexHeight;
	float fWidth	 = 0.0f;
	float fHeight	 = fRowHeight;
	
	while( *str ) {
		TCHAR c = *str++;
		
		if( c == _T('\n') ) {
			fRowWidth = 0.0f;
			fHeight  += fRowHeight;
		}
		
		if( (c-32) < 0 || (c-32) >= 128-32 )
			continue;
		
		float tx1 = mTexCoords[c-32][0];
		float tx2 = mTexCoords[c-32][2];
		
		fRowWidth += (tx2-tx1)*mTexWidth - 2*mSpacing;
		
		if( fRowWidth > fWidth )
			fWidth = fRowWidth;
	}
	
	resSize.cx = (int)fWidth;
	resSize.cy = (int)fHeight;
	
	return S_OK;
}
*/


/**
 *  Draws scaled 2D text. Note that x and y are in viewport coordinates
 *  (ranging from -1 to +1). xScale and yScale are the size fraction 
 *  relative to the entire viewport. For example, a xScale of 0.25 is
 *  1/8th of the screen width. This allows you to output text at a fixed
 *  fraction of the viewport, even if the screen or window size changes.
 */
/*
HRESULT CD3DFont::drawTextScaled(
	float x, float y, float z,
	float xScale, float yScale, D3DCOLOR color,
	const TCHAR* str, int flags )
{
	if( !mDevice )
		return E_FAIL;
	
	// Set up renderstate
	mBlockSaved->Capture();
	mBlockDrawText->Apply();
	mDevice->SetFVF( D3DFVF_FONT2DVERTEX );
	mDevice->SetPixelShader( NULL );
	mDevice->SetStreamSource( 0, mVB, 0, sizeof(SFont2DVertex) );
	
	// Set filter states
	if( flags & FILTERED ) {
		mDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		mDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	
	D3DVIEWPORT9 vp;
	mDevice->GetViewport( &vp );
	float lineHeight = ( mTexCoords[0][3] - mTexCoords[0][1] ) * mTexHeight;
	
	// Center the text block in the viewport
	if( flags & CENTERED_X ) {
		const TCHAR* strTmp = str;
		float xFinal = 0.0f;
		
		while( *strTmp ) {
			TCHAR c = *strTmp++;
			
			if( c == _T('\n') )
				break;	// Isn't supported.  
			if( (c-32) < 0 || (c-32) >= 128-32 )
				continue;
			
			float tx1 = mTexCoords[c-32][0];
			float tx2 = mTexCoords[c-32][2];
			
			float w = (tx2-tx1)*mTexWidth;
			
			w *= (xScale*vp.Height)/lineHeight;
			
			xFinal += w - (2 * mSpacing) * (xScale*vp.Height)/lineHeight;
		}
		
		x = -xFinal/vp.Width;
	}
	if( flags & CENTERED_Y ) {
		y = -lineHeight/vp.Height;
	}
	
	float sx  = (x+1.0f)*vp.Width/2;
	float sy  = (y+1.0f)*vp.Height/2;
	float sz  = z;
	float rhw = 1.0f;
	
	// Adjust for character spacing
	sx -= mSpacing * (xScale*vp.Height)/lineHeight;
	float startX = sx;
	
	// Fill vertex buffer
	SFont2DVertex* pVertices;
	int numTris = 0;
	mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
	
	while( *str ) {
		TCHAR c = *str++;
		
		if( c == _T('\n') ) {
			sx	= startX;
			sy += yScale*vp.Height;
		}
		
		if( (c-32) < 0 || (c-32) >= 128-32 )
			continue;
		
		float tx1 = mTexCoords[c-32][0];
		float ty1 = mTexCoords[c-32][1];
		float tx2 = mTexCoords[c-32][2];
		float ty2 = mTexCoords[c-32][3];
		
		float w = (tx2-tx1)*mTexWidth;
		float h = (ty2-ty1)*mTexHeight;
		
		w *= (xScale*vp.Height)/lineHeight;
		h *= (yScale*vp.Height)/lineHeight;
		
		if( c != _T(' ') ) {
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,sz,rhw), color, tx1, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,sz,rhw), color, tx1, ty1 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), color, tx2, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+0-0.5f,sz,rhw), color, tx2, ty1 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,sz,rhw), color, tx2, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,sz,rhw), color, tx1, ty1 );
			numTris += 2;
			
			if( numTris*3 > (MAX_NUM_VERTICES-6) ) {
				// Unlock, render, and relock the vertex buffer
				mVB->Unlock();
				mDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
				mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
				numTris = 0L;
			}
		}
		
		sx += w - (2 * mSpacing) * (xScale*vp.Height)/lineHeight;
	}
	
	// Unlock and render the vertex buffer
	mVB->Unlock();
	if( numTris > 0 )
		mDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
	
	// Restore the modified renderstates
	mBlockSaved->Apply();
	
	return S_OK;
}
*/

/** Draws 2D text. Note that sx and sy are in pixels. */
HRESULT CD3DFont::drawText( float sx, float sy, D3DCOLOR color, const TCHAR* str, int flags )
{
	CD3DDevice& device = CD3DDevice::getInstance();
	if( !device.isDevice() )
		return E_FAIL;
	
	// Setup renderstate
	mBlockSaved->Capture();
	mBlockDrawText->Apply();
	device.setDeclarationFVF( D3DFVF_FONT2DVERTEX );
	device.getDevice().SetPixelShader( NULL );
	device.getDevice().SetStreamSource( 0, mVB, 0, sizeof(SFont2DVertex) );
	
	// Set filter states
	if( flags & FILTERED ) {
		device.getDevice().SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		device.getDevice().SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	
	// Center the text block in the viewport
	if( flags & CENTERED_X ) {
		D3DVIEWPORT9 vp;
		device.getDevice().GetViewport( &vp );
		const TCHAR* strTmp = str;
		float xFinal = 0.0f;
		
		while( *strTmp ) {
			TCHAR c = *strTmp++;
			
			if( c == _T('\n') )
				break;	// Isn't supported.  
			if( (c-32) < 0 || (c-32) >= 128-32 )
				continue;
			
			float tx1 = mTexCoords[c-32][0];
			float tx2 = mTexCoords[c-32][2];
			
			float w = (tx2-tx1) *  mTexWidth / mTextScale;
			
			xFinal += w - (2 * mSpacing);
		}
		
		sx = (vp.Width-xFinal)/2.0f;
	}
	if( flags & CENTERED_Y ) {
		D3DVIEWPORT9 vp;
		device.getDevice().GetViewport( &vp );
		float lineHeight = ((mTexCoords[0][3]-mTexCoords[0][1])*mTexHeight);
		sy = (vp.Height-lineHeight)/2;
	}
	
	// Adjust for character spacing
	sx -= mSpacing;
	float startX = sx;
	
	// Fill vertex buffer
	SFont2DVertex* pVertices = NULL;
	int numTris = 0;
	mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
	
	while( *str ) {
		TCHAR c = *str++;
		
		if( c == _T('\n') ) {
			sx = startX;
			sy += (mTexCoords[0][3]-mTexCoords[0][1])*mTexHeight;
		}
		
		if( (c-32) < 0 || (c-32) >= 128-32 )
			continue;
		
		float tx1 = mTexCoords[c-32][0];
		float ty1 = mTexCoords[c-32][1];
		float tx2 = mTexCoords[c-32][2];
		float ty2 = mTexCoords[c-32][3];
		
		float w = (tx2-tx1) *  mTexWidth / mTextScale;
		float h = (ty2-ty1) * mTexHeight / mTextScale;
		
		if( c != _T(' ') ) {
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+h-0.5f,0.9f,1.0f), color, tx1, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,0.9f,1.0f), color, tx1, ty1 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), color, tx2, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+0-0.5f,0.9f,1.0f), color, tx2, ty1 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+w-0.5f,sy+h-0.5f,0.9f,1.0f), color, tx2, ty2 );
			*pVertices++ = gInitFont2DVertex( D3DXVECTOR4(sx+0-0.5f,sy+0-0.5f,0.9f,1.0f), color, tx1, ty1 );
			numTris += 2;
			
			if( numTris*3 > (MAX_NUM_VERTICES-6) ) {
				// Unlock, render, and relock the vertex buffer
				mVB->Unlock();
				device.getDevice().DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
				pVertices = NULL;
				mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
				numTris = 0L;
			}
		}
		
		sx += w - (2 * mSpacing);
	}
	
	// Unlock and render the vertex buffer
	mVB->Unlock();
	if( numTris > 0 )
		device.getDevice().DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
	
	// Restore the modified renderstates
	mBlockSaved->Apply();
	
	return S_OK;
}


/** Renders 3D text. */
/*
HRESULT CD3DFont::render3DText( const TCHAR* str, int flags )
{
	if( mDevice == NULL )
		return E_FAIL;
	
	// Setup renderstate
	mBlockSaved->Capture();
	mBlockDrawText->Apply();
	mDevice->SetFVF( D3DFVF_FONT3DVERTEX );
	mDevice->SetPixelShader( NULL );
	mDevice->SetStreamSource( 0, mVB, 0, sizeof(SFont3DVertex) );
	
	// Set filter states
	if( flags & FILTERED ) {
		mDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		mDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}
	
	// Position for each text element
	float x = 0.0f;
	float y = 0.0f;
	
	// Center the text block at the origin (not the viewport)
	if( flags & CENTERED_X ) {
		SIZE sz;
		getTextExtent( str, sz );
		x = -(((float)sz.cx)/10.0f)/2.0f;
	}
	if( flags & CENTERED_Y ) {
		SIZE sz;
		getTextExtent( str, sz );
		y = -(((float)sz.cy)/10.0f)/2.0f;
	}
	
	// Turn off culling for two-sided text
	if( flags & TWO_SIDED )
		mDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	
	// Adjust for character spacing
	x -= mSpacing / 10.0f;
	float startX = x;
	TCHAR c;
	
	// Fill vertex buffer
	SFont3DVertex* pVertices;
	int numTris = 0;
	mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
	
	while( (c = *str++) != 0 ) {
		if( c == '\n' ) {
			x = startX;
			y -= (mTexCoords[0][3]-mTexCoords[0][1])*mTexHeight/10.0f;
		}
		
		if( (c-32) < 0 || (c-32) >= 128-32 )
			continue;
		
		float tx1 = mTexCoords[c-32][0];
		float ty1 = mTexCoords[c-32][1];
		float tx2 = mTexCoords[c-32][2];
		float ty2 = mTexCoords[c-32][3];
		
		float w = (tx2-tx1) * mTexWidth  / ( 10.0f * mTextScale );
		float h = (ty2-ty1) * mTexHeight / ( 10.0f * mTextScale );
		
		if( c != _T(' ') ) {
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+0,y+0,0), D3DXVECTOR3(0,0,-1), tx1, ty2 );
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+w,y+h,0), D3DXVECTOR3(0,0,-1), tx2, ty1 );
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+w,y+0,0), D3DXVECTOR3(0,0,-1), tx2, ty2 );
			*pVertices++ = gInitFont3DVertex( D3DXVECTOR3(x+0,y+h,0), D3DXVECTOR3(0,0,-1), tx1, ty1 );
			numTris += 2;
			
			if( numTris*3 > (MAX_NUM_VERTICES-6) ) {
				// Unlock, render, and relock the vertex buffer
				mVB->Unlock();
				mDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
				mVB->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD );
				numTris = 0;
			}
		}
		
		x += w - (2 * mSpacing) / 10.0f;
	}
	
	// Unlock and render the vertex buffer
	mVB->Unlock();
	if( numTris > 0 )
		mDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numTris );
	
	// Restore the modified renderstates
	mBlockSaved->Apply();
	
	return S_OK;
}
*/
