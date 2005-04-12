// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __COLLISION_MESH_BUNDLE_H
#define __COLLISION_MESH_BUNDLE_H

#include "../collision/CollisionMesh.h"
#include "../utils/Singleton.h"
#include "StorageResourceBundle.h"

namespace dingus {


class CCollisionMeshBundle : public CStorageResourceBundle<CCollisionMesh>,
					public CSingleton<CCollisionMeshBundle>
{
protected:
	virtual CCollisionMesh* tryLoadResourceById( const CResourceId& id );
	virtual CCollisionMesh* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CCollisionMesh& resource ) { delete &resource; }
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CCollisionMeshBundle);
	CCollisionMeshBundle();
	virtual ~CCollisionMeshBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_COLMESH(rid) dingus::CCollisionMeshBundle::getInstance().getResourceById(rid)


#endif
