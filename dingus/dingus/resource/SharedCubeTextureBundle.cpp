// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SharedCubeTextureBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



void CSharedCubeTextureBundle::registerCubeTexture( const CResourceId& id, ICubeTextureCreator& creator )
{
	assert( &creator );
	IDirect3DCubeTexture9* tex = creator.createTexture();
	registerResource( id, *(new CD3DCubeTexture(tex)), &creator );
}


void CSharedCubeTextureBundle::createResource()
{
}

void CSharedCubeTextureBundle::activateResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		ICubeTextureCreator& creator = *it->second.first;
		assert( &creator );
		CD3DCubeTexture& res = *it->second.second;
		if( !res.isNull() )
			continue; // kind of HACK
		res.setObject( creator.createTexture() );
		assert( !res.isNull() );
	}
}

void CSharedCubeTextureBundle::passivateResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DCubeTexture& res = *it->second.second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}

void CSharedCubeTextureBundle::deleteResource()
{
}
