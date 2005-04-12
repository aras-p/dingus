// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "CubeTextureCreator.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../console/Console.h"

using namespace dingus;


IDirect3DCubeTexture9* CFixedCubeTextureCreator::createTexture()
{
	IDirect3DDevice9& dxdev = CD3DDevice::getInstance().getDevice();
	HRESULT hr;
	IDirect3DCubeTexture9* texture = NULL;

	// adjust params
	UINT size = mSize;
	UINT levels = mLevels;
	D3DFORMAT fmt = getFormat();
	D3DXCheckCubeTextureRequirements( &dxdev, &size, &levels, getUsage(), &fmt, getPool() );
	
	hr = CD3DDevice::getInstance().getDevice().CreateCubeTexture(
		size, levels,
		getUsage(), fmt, getPool(),
		&texture, NULL );
	if( FAILED( hr ) ) {
		std::string msg = "failed to create cube texture";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hr, msg );
	}
	assert( texture );
	return texture;
};

