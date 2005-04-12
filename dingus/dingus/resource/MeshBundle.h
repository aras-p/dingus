// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MESH_BUNDLE_H
#define __MESH_BUNDLE_H

#include "StorageResourceBundle.h"
#include "DeviceResource.h"
#include "../gfx/Mesh.h"
#include "../utils/Singleton.h"

namespace dingus {


class CMeshBundle :	public CStorageResourceBundle<CMesh>,
					public CSingleton<CMeshBundle>,
					public IDeviceResource
{
public:
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual CMesh* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CMesh& resource ) {
		if( resource.isCreated() )
			resource.deleteResource();
		delete &resource;
	}
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CMeshBundle);
	CMeshBundle();
	virtual ~CMeshBundle() { clear(); };
	
	/// @return false on not found
	bool loadMesh( const CResourceId& id, const CResourceId& fullName, CMesh& mesh ) const;
};


}; // namespace


/// Shortcut macro
#define RGET_MESH(rid) dingus::CMeshBundle::getInstance().getResourceById(rid)


#endif
