// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VolumeCreator.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../console/Console.h"

using namespace dingus;


IDirect3DVolumeTexture9* CFixedVolumeCreator::createTexture()
{
	IDirect3DDevice9& dxdev = CD3DDevice::getInstance().getDevice();
	HRESULT hr;
	IDirect3DVolumeTexture9* texture = NULL;

	// adjust params
	UINT width = mWidth;
	UINT height = mHeight;
	UINT depth = mDepth;
	UINT levels = mLevels;
	D3DFORMAT fmt = getFormat();
	D3DXCheckVolumeTextureRequirements( &dxdev, &width, &height, &depth, &levels, getUsage(), &fmt, getPool() );
	
	hr = CD3DDevice::getInstance().getDevice().CreateVolumeTexture(
		width, height, depth, levels,
		getUsage(), fmt, getPool(),
		&texture, NULL );
	if( FAILED( hr ) ) {
		std::string msg = "failed to create volume texture";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hr, msg );
	}
	assert( texture );
	return texture;
};
