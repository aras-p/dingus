// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SharedMeshBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



void CSharedMeshBundle::registerMesh( const CResourceId& id, IMeshCreator& creator )
{
	assert( &creator );
	CMesh* mesh = new CMesh();
	creator.createMesh( *mesh );
	registerResource( id, *mesh, &creator );
}


void CSharedMeshBundle::createResource()
{
	// recreate all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		IMeshCreator& creator = *it->second.first;
		assert( &creator );
		CMesh& res = *it->second.second;
		assert( !res.isCreated() );
		creator.createMesh( res );
		assert( res.isCreated() );
	}
}

void CSharedMeshBundle::activateResource()
{
}

void CSharedMeshBundle::passivateResource()
{
}

void CSharedMeshBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CMesh& res = *it->second.second;
		assert( res.isCreated() );
		res.deleteResource();
	}
}
