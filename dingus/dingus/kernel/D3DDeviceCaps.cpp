// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "D3DDeviceCaps.h"
#include <ddraw.h>

using namespace dingus;


// --------------------------------------------------------------------------


CD3DDeviceCaps::CD3DDeviceCaps()
:	mAdapterOrdinal(0), mDeviceType(D3DDEVTYPE_HAL),
	mHasShadowMaps(false),
	mHasFloatTextures(false),
	mVShaderVersion(VS_FFP),
	mPShaderVersion(PS_TSS),
	mVertexProcessing(VPCOUNT),
	mVideoMemoryMB(0), mCpuMhz(-1)
{
	memset( &mCaps, 0, sizeof(mCaps) );
	memset( &mAdapterID, 0, sizeof(mAdapterID) );
}

void CD3DDeviceCaps::setData( IDirect3D9& d3d, UINT adapterOrdinal, D3DDEVTYPE devType, const D3DCAPS9& caps )
{
	mAdapterOrdinal = adapterOrdinal;
	d3d.GetAdapterIdentifier( adapterOrdinal, 0, &mAdapterID );

	mCaps = caps;
	mDeviceType = devType;

	// --------------------------------
	//  get video memory size

	mVideoMemoryMB = calcVideoMemory();

	
	// --------------------------------
	//  get the CPU speed

	mCpuMhz = -1;
	HKEY hKey;
	LONG r = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey );
	if( r == ERROR_SUCCESS ) {
		DWORD size = sizeof(mCpuMhz);
		r = RegQueryValueEx( hKey, _T("~MHz"), NULL, NULL, (LPBYTE)&mCpuMhz, &size );
	}
	RegCloseKey( hKey );

	// --------------------------------
	//  shader versions

	// VS version
	DWORD vs = mCaps.VertexShaderVersion;
	if( vs >= D3DVS_VERSION(3,0) )
		mVShaderVersion = VS_3_0;
	else if( vs >= D3DVS_VERSION(2,0) )
		mVShaderVersion = VS_2_0;
	else if( vs >= D3DVS_VERSION(1,1) )
		mVShaderVersion = VS_1_1;
	else
		mVShaderVersion = VS_FFP;

	// PS version
	DWORD ps = mCaps.PixelShaderVersion;
	if( ps >= D3DPS_VERSION(3,0) )
		mPShaderVersion = PS_3_0;
	else if( ps >= D3DPS_VERSION(2,0) )
		mPShaderVersion = PS_2_0;
	else if( ps >= D3DPS_VERSION(1,4) )
		mPShaderVersion = PS_1_4;
	else if( ps >= D3DPS_VERSION(1,1) )
		mPShaderVersion = PS_1_1;
	else
		mPShaderVersion = PS_TSS;


	// --------------------------------
	//  check for special textures support

	// check for floating point rendertargets
	mHasFloatTextures = (
		mPShaderVersion >= PS_2_0
		&& SUCCEEDED( d3d.CheckDeviceFormat(
			adapterOrdinal, devType,
			D3DFMT_X8R8G8B8,
			D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F ) )
		&& SUCCEEDED( d3d.CheckDeviceFormat(
			adapterOrdinal, devType,
			D3DFMT_X8R8G8B8,
			D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F ) )
	);

	// check for native shadow maps
	mHasShadowMaps = (
		mPShaderVersion >= PS_1_1
		&& SUCCEEDED( d3d.CheckDeviceFormat(
			adapterOrdinal, devType,
			D3DFMT_X8R8G8B8,
			0, D3DRTYPE_TEXTURE, D3DFMT_D24S8 ) )
		&& SUCCEEDED( d3d.CheckDeviceFormat(
			adapterOrdinal, devType,
			D3DFMT_X8R8G8B8,
			0, D3DRTYPE_TEXTURE, D3DFMT_D16 ) )
	);
}


// --------------------------------------------------------------------------
//
// A rather ugly code to get the amount of video memory present on a D3D device
// Adapted from DX9 SDK October 2004, ConfigSystem sample
//

struct SDDEnumContext {
	struct SVideoDevice {
		GUID guid;
	    char DriverName[32];
	    DWORD MemorySize;
    };
	enum { MAX_DEVICES = 10 };

    SVideoDevice mAdapters[MAX_DEVICES];
	int		mAdapterCount;
};


static BOOL WINAPI gDDEnumCB( GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm )
{
    SDDEnumContext *ctx = (SDDEnumContext*)lpContext;
	
    if( ctx->mAdapterCount )
        memcpy( &ctx->mAdapters[ctx->mAdapterCount].guid, lpGUID, sizeof(GUID) );
    
	if( strlen( lpDriverName ) < 31 )
        strcpy( ctx->mAdapters[ctx->mAdapterCount].DriverName, lpDriverName );
    else
        ctx->mAdapters[ctx->mAdapterCount].DriverName[0] = 0;
	
    ctx->mAdapterCount++;
    if( ctx->mAdapterCount == SDDEnumContext::MAX_DEVICES )
        return false;
    else
        return true;
}


int CD3DDeviceCaps::calcVideoMemory()
{
	SDDEnumContext context;
	context.mAdapterCount = 0;


	// -----------------------
	// Get video memory size for all devices using ddraw 7 interfaces

	char ddrawPath[MAX_PATH];
	::GetSystemDirectoryA( ddrawPath, MAX_PATH );
	strcat( ddrawPath, "\\ddraw.dll" );
	IDirectDraw7* ddraw;
	HINSTANCE ddrawLib = LoadLibraryA( ddrawPath );
	if( ddrawLib ) {
		HRESULT (WINAPI* _DirectDrawCreateEx)( GUID* guid, void** dd, REFIID iid, IUnknown* unkOuter ) = (HRESULT (WINAPI*)( GUID* guid, void** dd, REFIID iid, IUnknown* unkOuter ))GetProcAddress( ddrawLib, "DirectDrawCreateEx" );
		if( _DirectDrawCreateEx ) {
			HRESULT (WINAPI* _DirectDrawEnumerateEx)( LPDDENUMCALLBACKEXA, LPVOID, DWORD ) = (HRESULT (WINAPI*)( LPDDENUMCALLBACKEXA, LPVOID, DWORD )) GetProcAddress( ddrawLib, "DirectDrawEnumerateExA" );
			if( _DirectDrawEnumerateEx ) {

								
				// DirectDraw 7 will enumerate 1 device (NULL). Or a NULL device and then a GUID for each adapter.
				_DirectDrawEnumerateEx( gDDEnumCB, &context, DDENUM_ATTACHEDSECONDARYDEVICES );
				
				// Create a DirectDraw object for the device specified
				for( int t0 = 0; t0 < context.mAdapterCount; ++t0 ) {
					HRESULT result = _DirectDrawCreateEx( t0 ? &context.mAdapters[t0].guid : NULL, (void**)&ddraw, IID_IDirectDraw7, NULL );
					if( SUCCEEDED(result) ) {
						ddraw->SetCooperativeLevel( NULL, DDSCL_NORMAL );
						
						// See how much memory is on the card
						DDSCAPS2 ddscaps2;
						DWORD junk;
						DWORD videoMemory = (DWORD)-1;
						DWORD temp = 0;
						memset( &ddscaps2, 0, sizeof(ddscaps2) );
						ddscaps2.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
						ddraw->GetAvailableVidMem( &ddscaps2, &temp, &junk );
						if( temp > 0 && temp < videoMemory )
							videoMemory=temp;
						ddscaps2.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
						ddraw->GetAvailableVidMem( &ddscaps2, &temp, &junk );
						if( temp > 0 && temp < videoMemory )
							videoMemory=temp;
						ddscaps2.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
						ddraw->GetAvailableVidMem( &ddscaps2, &temp, &junk );
						if( temp > 0 && temp < videoMemory )
							videoMemory=temp;
						ddscaps2.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
						ddraw->GetAvailableVidMem( &ddscaps2, &temp, &junk );
						if( temp > 0 && temp < videoMemory )
							videoMemory=temp;

						// If cannot get memory size, there must be a problem with D3D / DirectDraw
						if( (DWORD)-1 == videoMemory ) {
							videoMemory = 0;
						} else {
							
							// Round the video memory number depending upon the size
							if( videoMemory<=16*1024*1024 ) {
								videoMemory=((videoMemory+8*1024*1024-1) & ~(8*1024*1024-1));				// Round up to nearest 8 under 20Meg
							} else {
								if( videoMemory <= 64*1024*1024 )
									videoMemory=((videoMemory+32*1024*1024-1) & ~(32*1024*1024-1)); 		// Round to neaest 32 Meg under 64Meg
								else
									videoMemory=((videoMemory+64*1024*1024-1) & ~(64*1024*1024-1)); 		// Round to neaest 64 Meg over 64Meg
							}
						}
						
						// Clean up ddraw7
						context.mAdapters[t0].MemorySize = videoMemory;
						ddraw->Release();
					}
				}
			}
		}
		
		// Release ddraw library
		FreeLibrary( ddrawLib );
	}
	
	// -----------------------
	// Find the display adapter (ddraw and d3d9 do not enumerate the same order)

	int videoMemory = 0;
	if( context.mAdapterCount > 0 )
		videoMemory = context.mAdapters[0].MemorySize;

	for( int i = 0; i < context.mAdapterCount; ++i ) {
		if( !stricmp( context.mAdapters[i].DriverName, mAdapterID.DeviceName ) )
			videoMemory = context.mAdapters[i].MemorySize;
	}
	return videoMemory >> 20;
}
