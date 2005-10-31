// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "TextureBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../utils/StringHelper.h"

using namespace dingus;


CTextureBundle::CTextureBundle()
{
	addExtension( ".dds" );
	addExtension( ".png" );
	addExtension( ".jpg" );
	addExtension( ".tga" );
	addExtension( ".hdr" );
	addExtension( ".bmp" );
}

IDirect3DTexture9* CTextureBundle::loadTexture( const CResourceId& id, const CResourceId& fullName ) const
{
	IDirect3DTexture9* texture = NULL;

	// load mips from file in DDS case; construct full mip chain for other
	// formats
	DWORD mipLevels = 0;
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".dds" ) )
		mipLevels = D3DX_FROM_FILE;

	HRESULT hres = D3DXCreateTextureFromFileEx(
		&CD3DDevice::getInstance().getDevice(),
		fullName.getUniqueName().c_str(),
		D3DX_DEFAULT, D3DX_DEFAULT,
		mipLevels,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		NULL,
		NULL,
		&texture );
	
	if( !SUCCEEDED( hres ) )
		return NULL;

	assert( texture );
	CONSOLE.write( "texture loaded '" + id.getUniqueName() + "'" );
	return texture;
}

CD3DTexture* CTextureBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	IDirect3DTexture9* texture = loadTexture( id, fullName );
	if( !texture )
		return NULL;
	return new CD3DTexture( texture );
}

void CTextureBundle::createResource()
{
	// reload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DTexture& res = *it->second;
		assert( res.isNull() );
		CD3DTexture* n = tryLoadResourceById( it->first );
		assert( n );
		res.setObject( n->getObject() );
		delete n;
		assert( !res.isNull() );
	}
}

void CTextureBundle::activateResource()
{
}

void CTextureBundle::passivateResource()
{
}

void CTextureBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DTexture& res = *it->second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}

