#include "stdafx.h"
#include "d3d.h"


TD3DPtr			gD3D;
TD3DDevicePtr	gD3DDevice;


void initD3D()
{
	gD3D = TD3DPtr( Direct3DCreate9( D3D_SDK_VERSION ) );
	if( !gD3D )
		throw std::runtime_error("Failed to create D3D");

	D3DDISPLAYMODE mode;
	if( FAILED( gD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &mode ) ) )
		throw std::runtime_error("Failed to get display mode");

	D3DPRESENT_PARAMETERS params;
	ZeroMemory( &params, sizeof(params) );
	params.BackBufferWidth = 1;
	params.BackBufferHeight = 1;
	params.BackBufferFormat = mode.Format;
	params.BackBufferCount = 1;
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_COPY;

	IDirect3DDevice9* dev = NULL;
	if( FAILED( gD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetConsoleWindow(),
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &dev ) ) )
		throw std::runtime_error("Failed to create D3D device");
	assert( dev );
	gD3DDevice = TD3DDevicePtr( dev );
}

void closeD3D()
{
	gD3DDevice = NULL;
	gD3D = NULL;
}
