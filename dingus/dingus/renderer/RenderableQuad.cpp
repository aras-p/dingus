// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableQuad.h"
#include "RenderContext.h"
#include "../kernel/D3DDevice.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"

using namespace dingus;


DEFINE_POOLED_ALLOC(dingus::CRenderableQuad,64,false);

namespace {
	// Full-screen quad vertex format
	struct SVertexScreen
	{
		SVector4 p;
		SVector2 t;
		static const DWORD FVF;
	};
	const DWORD SVertexScreen::FVF = D3DFVF_XYZRHW | D3DFVF_TEX1;
}; // namespace


// --------------------------------------------------------------------------
//  CRenderableQuad
// --------------------------------------------------------------------------

CRenderableQuad::CRenderableQuad( const renderquad::SCoordRect& uvRect, int priority )
:	CRenderable( NULL, priority )
,	mUVRect( uvRect )
{
}


void CRenderableQuad::render( CRenderContext const& ctx )
{
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	
	// acquire render target width and height
	IDirect3DSurface9* rt = device.getRenderTarget( 0 );
	assert( rt );
	D3DSURFACE_DESC rtDesc;
	rt->GetDesc( &rtDesc );
	
	// ensure that we're directly mapping texels to pixels by offset by 0.5
	float rtW5 = (float)rtDesc.Width - 0.5f;
	float rtH5 = (float)rtDesc.Height - 0.5f;
	
	// Draw the quad
	SVertexScreen svQuad[4];
	
	svQuad[0].p.set(-0.5f, -0.5f, 0.5f, 1.0f);
	svQuad[0].t.set(mUVRect.u1, mUVRect.v1);
	
	svQuad[1].p.set(rtW5, -0.5f, 0.5f, 1.0f);
	svQuad[1].t.set(mUVRect.u2, mUVRect.v1);
	
	svQuad[2].p.set(-0.5f, rtH5, 0.5f, 1.0f);
	svQuad[2].t.set(mUVRect.u1, mUVRect.v2);
	
	svQuad[3].p.set(rtW5, rtH5, 0.5f, 1.0f);
	svQuad[3].t.set(mUVRect.u2, mUVRect.v2);
	
	device.setDeclarationFVF(SVertexScreen::FVF);
	dx.DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof(SVertexScreen) );
	device.setVertexBuffer( 0, NULL, 0, 0 );
	
	// stats
	stats.incDrawCalls();
	stats.incVerticesRendered( 4 );
	stats.incPrimsRendered( 2 );
}



/*
HRESULT GetTextureRect( PDIRECT3DTEXTURE9 pTexture, RECT* pRect )
{
    HRESULT hr = S_OK;

    if( pTexture == NULL || pRect == NULL )
        return E_INVALIDARG;

    D3DSURFACE_DESC desc;
    hr = pTexture->GetLevelDesc( 0, &desc );
    if( FAILED(hr) )
        return hr;

    pRect->left = 0;
    pRect->top = 0;
    pRect->right = desc.Width;
    pRect->bottom = desc.Height;

    return S_OK;
}
*/


void renderquad::calcCoordRect(
	CD3DTexture& srcTex, const RECT* srcRect,
	CD3DTexture& dstTex, const RECT* dstRect,
	SCoordRect& coords )
{
	assert( srcTex.getObject() != NULL );
	assert( dstTex.getObject() != NULL );
	
	D3DSURFACE_DESC desc;
	float tU, tV;
	
	// start with a default mapping of the complete source surface to
	// complete destination surface
	coords.u1 = 0.0f;
	coords.v1 = 0.0f;
	coords.u2 = 1.0f; 
	coords.v2 = 1.0f;
	
	// if not using the complete source surface, adjust the coordinates
	if( srcRect != NULL )
	{
		// get source texture description
		HRESULT hr = srcTex.getObject()->GetLevelDesc( 0, &desc );
		if( FAILED(hr) ) {
			assert( false );
			return;
		}
		
		// distance between source texel centers in texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;
		
		coords.u1 += srcRect->left * tU;
		coords.v1 += srcRect->top * tV;
		coords.u2 -= (desc.Width - srcRect->right) * tU;
		coords.v2 -= (desc.Height - srcRect->bottom) * tV;
	}
	
	// If not drawing to the complete destination surface, adjust the coordinates
	if( dstRect != NULL )
	{
		// get destination texture description
		HRESULT hr = dstTex.getObject()->GetLevelDesc( 0, &desc );
		if( FAILED(hr) ) {
			assert( false );
			return;
		}
		
		// distance between source texel centers in texture address space
		tU = 1.0f / desc.Width;
		tV = 1.0f / desc.Height;
		
		coords.u1 -= dstRect->left * tU;
		coords.v1 -= dstRect->top * tV;
		coords.u2 += (desc.Width - dstRect->right) * tU;
		coords.v2 += (desc.Height - dstRect->bottom) * tV;
	}
}
