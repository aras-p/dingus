// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SKINMESH_BUNDLE_H
#define __SKINMESH_BUNDLE_H

#include "StorageResourceBundle.h"
#include "DeviceResource.h"
#include "../gfx/skeleton/SkinMesh.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSkinMeshBundle :	public CStorageResourceBundle<CSkinMesh>,
					public CSingleton<CSkinMeshBundle>,
					public IDeviceResource
{
public:
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual CSkinMesh* tryLoadResourceById( const CResourceId& id );
	virtual CSkinMesh* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CSkinMesh& resource ) {
		if( resource.isCreated() )
			resource.deleteResource();
		delete &resource;
	}
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CSkinMeshBundle);
	CSkinMeshBundle();
	virtual ~CSkinMeshBundle() { clear(); };
	
	/// @return false on not found
	bool loadMesh( const CResourceId& id, const CResourceId& fullName, CSkinMesh& mesh ) const;
};


}; // namespace


/// Shortcut macro
#define RGET_SKIN(rid) dingus::CSkinMeshBundle::getInstance().getResourceById(rid)


#endif
