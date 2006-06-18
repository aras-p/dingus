// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "CubeTextureBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


CCubeTextureBundle::CCubeTextureBundle()
{
	addExtension( ".dds" );
	addExtension( ".png" );
	addExtension( ".jpg" );
	addExtension( ".tga" );
	addExtension( ".hdr" );
	addExtension( ".bmp" );
};

namespace {

// If last thing before file extension is "_px",
// then load "_px", "_nx", "_py", "_ny", "_pz" and "_nz" into
// corresponding cube map faces.
IDirect3DCubeTexture9*	loadCubemapSeparateImages( const std::string& fileName )
{
	IDirect3DCubeTexture9* texture = NULL;

	size_t dotIdx = fileName.find_last_of( '.' );
	if( dotIdx != std::string::npos && dotIdx >= 3 ) { // fit "_px"
		std::string beforeExt = fileName.substr( dotIdx-3, 3 );
		if( beforeExt == std::string("_px") ) {
			D3DXIMAGE_INFO info;
			HRESULT hres = D3DXGetImageInfoFromFile( fileName.c_str(), &info );
			if( !SUCCEEDED(hres) )
				return NULL;
			if( info.Width != info.Height ) // cube map faces must be square
				return NULL;
			hres = D3DXCreateCubeTexture(
				&CD3DDevice::getInstance().getDevice(), info.Width, 0, 0, info.Format, D3DPOOL_MANAGED, &texture );
			if( !SUCCEEDED(hres) )
				return NULL;
			static const char* CHAR1 = "pnpnpn";
			static const char* CHAR2 = "xxyyzz";
			std::string fileNameCopy = fileName;
			size_t charIdx = dotIdx-2;
			for( int i = 0; i < 6; ++i ) {
				IDirect3DSurface9* surface = NULL;
				hres = texture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &surface );
				if( !SUCCEEDED(hres) ) {
					texture->Release();
					return NULL;
				}
				fileNameCopy[charIdx] = CHAR1[i];
				fileNameCopy[charIdx+1] = CHAR2[i];
				hres = D3DXLoadSurfaceFromFile( surface, NULL, NULL, fileNameCopy.c_str(), NULL, D3DX_DEFAULT, 0, NULL );
				surface->Release();
				// NOTE: don't fail, just the cube map faces will contain garbage
				//if( !SUCCEEDED(hres) ) goto _fail;
			}
			// generate mip maps
			D3DXFilterTexture( texture, NULL, 0, D3DX_DEFAULT );
		}
	}
	return texture; // may be NULL
}


static int bytesPerFormat( D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_R8G8B8      : return 3;
	case D3DFMT_A8R8G8B8    : return 4;
	case D3DFMT_X8R8G8B8    : return 4;
	case D3DFMT_R5G6B5      : return 2;
	case D3DFMT_X1R5G5B5    : return 2;
	case D3DFMT_A1R5G5B5    : return 2;
	case D3DFMT_A4R4G4B4    : return 2;
	case D3DFMT_R3G3B2      : return 1;
	case D3DFMT_A8          : return 1;
	case D3DFMT_A8R3G3B2    : return 2;
	case D3DFMT_X4R4G4B4    : return 2;
	case D3DFMT_A8B8G8R8    : return 4;
	case D3DFMT_X8B8G8R8    : return 4;
	case D3DFMT_G16R16      : return 4;
	case D3DFMT_A2R10G10B10 : return 4;
	case D3DFMT_A16B16G16R16: return 8;
	case D3DFMT_L16         : return 2;
	case D3DFMT_R16F        : return 2;
	case D3DFMT_G16R16F     : return 4;
	case D3DFMT_A16B16G16R16F : return 8;
	case D3DFMT_R32F          : return 4;
	case D3DFMT_G32R32F       : return 8;
	case D3DFMT_A32B32G32R32F : return 16;
	default: return -1;
	}
}


// If given image has 3x4 aspect ratio, try loading it
// as vertical cross.
IDirect3DCubeTexture9*	loadCubemapVerticalCross( const std::string& fileName )
{
	IDirect3DCubeTexture9* texture = NULL;

	D3DXIMAGE_INFO info;
	HRESULT hres = D3DXGetImageInfoFromFile( fileName.c_str(), &info );
	if( !SUCCEEDED(hres) )
		return NULL;
	// aspect must be 3x4, sizes divisible by 3 and 4
	if( (info.Width%3)!=0 || (info.Height%4)!=0 || info.Width/3!=info.Height/4 )
		return NULL;
	// we must recognize the format
	int bytesPerTexel = bytesPerFormat( info.Format );
	if( bytesPerTexel == -1 )
		return NULL;

	const int faceSize = info.Width / 3;

	// load the image into off-screen surface
	IDirect3DSurface9* srcSurface = 0;
	CD3DDevice& dx = CD3DDevice::getInstance();
	hres = dx.getDevice().CreateOffscreenPlainSurface( info.Width, info.Height, info.Format, D3DPOOL_SYSTEMMEM, &srcSurface, 0 );
	if( FAILED(hres) )
		return NULL;
	hres = D3DXLoadSurfaceFromFile( srcSurface, NULL, NULL, fileName.c_str(), NULL, D3DX_DEFAULT, 0, NULL );
	if( FAILED(hres) ) {
		srcSurface->Release();
		return NULL;
	}
	// flip the lowest face upside-down and left-to-right
	{
		D3DLOCKED_RECT lr;
		RECT rc;
		rc.left = faceSize;
		rc.right = faceSize*2;
		rc.top = faceSize*3;
		rc.bottom = info.Height;

		srcSurface->LockRect( &lr, &rc, 0 );
		BYTE* ptr = (BYTE*)lr.pBits;
		BYTE* tmp = new BYTE[bytesPerTexel*(faceSize+1)]; // last is temp texel
		BYTE* tmpTexel = tmp + bytesPerTexel*faceSize;
		for( int y = 0; y < faceSize/2; ++y ) {
			int x;
			BYTE* p1 = ptr + y*lr.Pitch;
			BYTE* p2 = ptr + (faceSize-1-y)*lr.Pitch;
			memcpy( tmp, p1, bytesPerTexel*faceSize );
			memcpy( p1, p2, bytesPerTexel*faceSize );
			for( x = 0; x < faceSize/2; ++x ) {
				BYTE* tp1 = p1 + x*bytesPerTexel;
				BYTE* tp2 = p1 + (faceSize-1-x)*bytesPerTexel;
				memcpy( tmpTexel, tp1, bytesPerTexel );
				memcpy( tp1, tp2, bytesPerTexel );
				memcpy( tp2, tmpTexel, bytesPerTexel );
			}
			memcpy( p2, tmp, bytesPerTexel*faceSize );
			for( x = 0; x < faceSize/2; ++x ) {
				BYTE* tp1 = p2 + x*bytesPerTexel;
				BYTE* tp2 = p2 + (faceSize-1-x)*bytesPerTexel;
				memcpy( tmpTexel, tp1, bytesPerTexel );
				memcpy( tp1, tp2, bytesPerTexel );
				memcpy( tp2, tmpTexel, bytesPerTexel );
			}
		}
		delete[] tmp;
		srcSurface->UnlockRect();
	}

	// create cube map
	D3DXCreateCubeTexture( &dx.getDevice(), faceSize, 1 /* TBD */, 0, info.Format, D3DPOOL_MANAGED, &texture );
	if( FAILED(hres) ) {
		srcSurface->Release();
		texture->Release();
		return NULL;
	}

	// copy source cross portions to corresponding cube faces
	// px, nx, py, ny, pz, nz
	static int FACE_X[6] = { 2, 0, 1, 1, 1, 1, };
	static int FACE_Y[6] = { 1, 1, 0, 2, 1, 3, };

	for( int i = 0; i < 6; ++i ) {
		IDirect3DSurface9* surface = NULL;
		hres = texture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &surface );
		if( !SUCCEEDED(hres) ) {
			srcSurface->UnlockRect();
			srcSurface->Release();
			texture->Release();
			return NULL;
		}
		RECT srcRC;
		srcRC.left = FACE_X[i] * faceSize;
		srcRC.right = srcRC.left + faceSize;
		srcRC.top = FACE_Y[i] * faceSize;
		srcRC.bottom = srcRC.top + faceSize;

		hres = D3DXLoadSurfaceFromSurface( surface, NULL, NULL, srcSurface, NULL, &srcRC, D3DX_DEFAULT, 0 );
		surface->Release();
		// NOTE: don't fail, just the cube map faces will contain garbage
		//if( !SUCCEEDED(hres) ) goto _fail;
	}
	srcSurface->Release();
	// generate mip maps
	D3DXFilterTexture( texture, NULL, 0, D3DX_DEFAULT );
	return texture;
}

}; // anonymous namespace


IDirect3DCubeTexture9* CCubeTextureBundle::loadTexture( const CResourceId& id, const CResourceId& fullName ) const
{
	IDirect3DCubeTexture9* texture = NULL;
	// try to load from native cube texture format
	HRESULT hres = D3DXCreateCubeTextureFromFileEx(
		&CD3DDevice::getInstance().getDevice(),
		fullName.getUniqueName().c_str(),
		D3DX_DEFAULT,
		0,	// mipLevels
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		NULL,
		NULL,
		&texture );
	
	if( !SUCCEEDED( hres ) ) {
		// if we didn't load cube map directly, try
		// separate faces
		texture = loadCubemapSeparateImages( fullName.getUniqueName() );
		if( texture )
			goto _ok;

		// try vertical cube
		texture = loadCubemapVerticalCross( fullName.getUniqueName() );
		if( texture )
			goto _ok;

		// no way to load it
		return NULL;
	}

_ok:
	assert( texture );
	CONSOLE.write( "cubemap loaded '" + id.getUniqueName() + "'" );
	return texture;
}

CD3DCubeTexture* CCubeTextureBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	IDirect3DCubeTexture9* texture = loadTexture( id, fullName );
	if( !texture )
		return NULL;
	return new CD3DCubeTexture( texture );
}

void CCubeTextureBundle::createResource()
{
	// reload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DCubeTexture& res = *it->second;
		assert( res.isNull() );
		CD3DCubeTexture* n = tryLoadResourceById( it->first );
		assert( n );
		res.setObject( n->getObject() );
		delete n;
		assert( !res.isNull() );
	}
}

void CCubeTextureBundle::activateResource()
{
}

void CCubeTextureBundle::passivateResource()
{
}

void CCubeTextureBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DCubeTexture& res = *it->second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}

