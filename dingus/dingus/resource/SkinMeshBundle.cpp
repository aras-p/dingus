// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "SkinMeshBundle.h"
#include "MeshBundle.h"
#include "SkeletonInfoBundle.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


CSkinMeshBundle::CSkinMeshBundle()
{
};


bool CSkinMeshBundle::loadMesh( const CResourceId& id, const CResourceId& fullName, CSkinMesh& mesh ) const
{
	CMesh* srcMesh = RGET_MESH(id);
	CSkeletonInfo* skeleton = RGET_SKEL(id);
	mesh.createResource( *srcMesh, *skeleton );
	CONSOLE.write( "skinned mesh loaded '" + id.getUniqueName() + "'" );
	return true;
}

CSkinMesh* CSkinMeshBundle::tryLoadResourceById( const CResourceId& id )
{
	// just loadResourceById - it uses MeshBundle/SkeletonInfoBundle for actual
	// loading, so no predir or extension is required here
	return loadResourceById( id, id );
}


CSkinMesh* CSkinMeshBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	CSkinMesh* mesh = new CSkinMesh();
	bool ok = loadMesh( id, fullName, *mesh );
	if( !ok ) {
		delete mesh;
		return NULL;
	}
	return mesh;
}

void CSkinMeshBundle::createResource()
{
	// reload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CSkinMesh& res = *it->second;
		assert( !res.isCreated() );
		loadMesh( it->first, it->first, res );
		assert( res.isCreated() );
	}
}

void CSkinMeshBundle::activateResource()
{
}

void CSkinMeshBundle::passivateResource()
{
}

void CSkinMeshBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CSkinMesh& res = *it->second;
		assert( res.isCreated() );
		res.deleteResource();
	}
}
