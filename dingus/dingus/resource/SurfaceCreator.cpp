// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SurfaceCreator.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../console/Console.h"

using namespace dingus;

IDirect3DSurface9* CAbstractSurfaceCreator::internalCreate( int width, int height ) const
{
	HRESULT hres;
	IDirect3DSurface9* surface = NULL;

	CD3DDevice& dx = CD3DDevice::getInstance();
	D3DMULTISAMPLE_TYPE msType = mMultiSampleType;
	DWORD msQty = mMultiSampleQuality;
		
	if( mDepthStencil ) {
		if( mUseScreenMultiSample ) {
			msType = dx.getMainZStencilDesc().MultiSampleType;
			msQty = dx.getMainZStencilDesc().MultiSampleQuality;
		}
		hres = dx.getDevice().CreateDepthStencilSurface(
			width, height, mFormat, msType, msQty, FALSE, &surface, NULL );
	} else {
		if( mUseScreenMultiSample ) {
			msType = dx.getBackBufferDesc().MultiSampleType;
			msQty = dx.getBackBufferDesc().MultiSampleQuality;
		}
		hres = dx.getDevice().CreateRenderTarget(
			width, height, mFormat, msType, msQty, FALSE, &surface, NULL );
	}
	if( FAILED( hres ) ) {
		char buf[1000];
		sprintf( buf,
			"AbstractSurfaceCreator: failed to create surface \n"
			"(type=%s w=%i h=%i fmt=%i msaa=%i/%i)",
			mDepthStencil ? "DS" : "RT", width, height, mFormat, msType, msQty );
		std::string msg = buf;
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hres, msg );
	}
	assert( surface );
	return surface;
}

IDirect3DSurface9* CFixedSurfaceCreator::createSurface()
{
	return internalCreate( mWidth, mHeight );
};


IDirect3DSurface9* CScreenBasedSurfaceCreator::createSurface()
{
	CD3DDevice& device = CD3DDevice::getInstance();
	int width = int( device.getBackBufferWidth() * mWidthFactor );
	int height = int( device.getBackBufferHeight() * mHeightFactor );
	return internalCreate( width, height );
};


IDirect3DSurface9* CScreenBasedPow2SurfaceCreator::createSurface()
{
	CD3DDevice& device = CD3DDevice::getInstance();
	int width = int( device.getBackBufferWidth() * mWidthFactor );
	int height = int( device.getBackBufferHeight() * mHeightFactor );
	
	// lower width/height to be power-of-2
	int hibit;
	hibit = 0;
	while( width>>hibit != 0 )
		++hibit;
	width &= (1<<(hibit-1));
	hibit = 0;
	while( height>>hibit != 0 )
		++hibit;
	height &= (1<<(hibit-1));

	return internalCreate( width, height );
};

IDirect3DSurface9* CTextureLevelSurfaceCreator::createSurface()
{
	HRESULT hres;
	IDirect3DSurface9* surface = NULL;

	assert( mTexture );
	assert( mTexture->getObject() );

	hres = mTexture->getObject()->GetSurfaceLevel( mLevel, &surface );
	if( FAILED( hres ) ) {
		std::string msg = "TextureLevelSurfaceCreator: failed to get texture level";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hres, msg );
	}
	assert( surface );
	return surface;
};
