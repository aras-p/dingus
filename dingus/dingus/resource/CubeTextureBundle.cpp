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
	addExtension( ".bmp" );
};

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
		// now, if we didn't load, and last thing before file extension is
		// "_px", then load "_px", "_nx", "_py", "_ny", "_pz" and "_nz" into
		// corresponding faces.
		int dotIdx = fullName.getUniqueName().find_last_of( '.' );
		if( dotIdx >= 3 ) { // fit "_px"
			std::string beforeExt = fullName.getUniqueName().substr( dotIdx-3, 3 );
			if( beforeExt == std::string("_px") ) {
				D3DXIMAGE_INFO info;
				hres = D3DXGetImageInfoFromFile( fullName.getUniqueName().c_str(), &info );
				if( !SUCCEEDED(hres) ) goto _fail;
				hres = D3DXCreateCubeTexture(
					&CD3DDevice::getInstance().getDevice(), info.Width, 0, 0, info.Format, D3DPOOL_MANAGED, &texture );
				if( !SUCCEEDED(hres) ) goto _fail;
				static const char* CHAR1 = "pnpnpn";
				static const char* CHAR2 = "xxyyzz";
				std::string fileName = fullName.getUniqueName();
				int charIdx = dotIdx-2;
				for( int i = 0; i < 6; ++i ) {
					IDirect3DSurface9* surface = NULL;
					hres = texture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, 0, &surface );
					if( !SUCCEEDED(hres) ) goto _fail;
					fileName[charIdx] = CHAR1[i];
					fileName[charIdx+1] = CHAR2[i];
					hres = D3DXLoadSurfaceFromFile( surface, NULL, NULL, fileName.c_str(), NULL, D3DX_DEFAULT, 0, NULL );
					surface->Release();
					// don't fail, just the cubemap faces will contain garbage
					//if( !SUCCEEDED(hres) ) goto _fail;
				}
				// gen mipmaps
				D3DXFilterTexture( texture, NULL, 0, D3DX_DEFAULT );
				goto _ok;
			}
		}

_fail:
		std::string msg = "failed to load cubemap '" + fullName.getUniqueName() + "'";
		CConsole::CON_ERROR.write(msg);
		THROW_DXERROR( hres, msg );
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

