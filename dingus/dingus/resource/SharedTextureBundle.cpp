// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SharedTextureBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



void CSharedTextureBundle::registerTexture( CResourceId const& id, ITextureCreator& creator )
{
	assert( &creator );
	IDirect3DTexture9* tex = creator.createTexture();
	CD3DTexture* texPrx = new CD3DTexture( tex );

	registerResource( id, *texPrx, &creator );
}


void CSharedTextureBundle::createResource()
{
}

void CSharedTextureBundle::activateResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		ITextureCreator& creator = *it->second.first;
		assert( &creator );
		CD3DTexture& res = *it->second.second;
		if( !res.isNull() )
			continue; // kind of HACK
		res.setObject( creator.createTexture() );
		assert( !res.isNull() );
	}
}

void CSharedTextureBundle::passivateResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DTexture& res = *it->second.second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}

void CSharedTextureBundle::deleteResource()
{
}
