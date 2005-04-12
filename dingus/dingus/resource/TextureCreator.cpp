// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "TextureCreator.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../console/Console.h"

using namespace dingus;


IDirect3DTexture9* CFixedTextureCreator::createTexture()
{
	IDirect3DDevice9& dxdev = CD3DDevice::getInstance().getDevice();
	HRESULT hr;
	IDirect3DTexture9* texture = NULL;

	// adjust params
	UINT width = mWidth;
	UINT height = mHeight;
	UINT levels = mLevels;
	D3DFORMAT fmt = getFormat();
	D3DXCheckTextureRequirements( &dxdev, &width, &height, &levels, getUsage(), &fmt, getPool() );
	
	hr = CD3DDevice::getInstance().getDevice().CreateTexture(
		width, height, levels,
		getUsage(), fmt, getPool(),
		&texture, NULL );
	if( FAILED( hr ) ) {
		std::string msg = "failed to create texture";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hr, msg );
	}
	assert( texture );
	return texture;
};


IDirect3DTexture9* CScreenBasedTextureCreator::createTexture()
{
	HRESULT hr;
	IDirect3DTexture9* texture = NULL;

	CD3DDevice& device = CD3DDevice::getInstance();
	int width = int( device.getBackBufferWidth() * mWidthFactor );
	int height = int( device.getBackBufferHeight() * mHeightFactor );
	
	hr = device.getDevice().CreateTexture(
		width, height, mLevels,
		getUsage(), getFormat(), getPool(),
		&texture, NULL );
	if( FAILED( hr ) ) {
		std::string msg = "failed to create texture";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hr, msg );
	}
	assert( texture );
	return texture;
};


IDirect3DTexture9* CScreenBasedPow2TextureCreator::createTexture()
{
	HRESULT hr;
	IDirect3DTexture9* texture = NULL;

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
	
	hr = device.getDevice().CreateTexture(
		width, height, mLevels,
		getUsage(), getFormat(), getPool(),
		&texture, NULL );
	if( FAILED( hr ) ) {
		std::string msg = "failed to create texture";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hr, msg );
	}
	assert( texture );
	return texture;
};
