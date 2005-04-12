// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SharedVolumeBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



void CSharedVolumeBundle::registerVolume( CResourceId const& id, IVolumeCreator& creator )
{
	assert( &creator );
	IDirect3DVolumeTexture9* tex = creator.createTexture();
	registerResource( id, *(new CD3DVolumeTexture(tex)), &creator );
}


void CSharedVolumeBundle::createResource()
{
}

void CSharedVolumeBundle::activateResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		IVolumeCreator& creator = *it->second.first;
		assert( &creator );
		CD3DVolumeTexture& res = *it->second.second;
		if( !res.isNull() )
			continue; // kind of HACK
		res.setObject( creator.createTexture() );
		assert( !res.isNull() );
	}
}

void CSharedVolumeBundle::passivateResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CD3DVolumeTexture& res = *it->second.second;
		assert( !res.isNull() );
		res.getObject()->Release();
		res.setObject( NULL );
	}
}

void CSharedVolumeBundle::deleteResource()
{
}
