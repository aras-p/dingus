// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SHARED_MESH_BUNDLE_H
#define __SHARED_MESH_BUNDLE_H

#include "SharedResourceBundle.h"
#include "MeshCreator.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSharedMeshBundle : public CSharedResourceBundle<CMesh,IMeshCreator::TSharedPtr>,
							public CSingleton<CSharedMeshBundle>,
							public IDeviceResource
{
public:
	void registerMesh( const CResourceId& id, IMeshCreator& creator );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CMesh& resource ) {
		if( resource.isCreated() )
			resource.deleteResource();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSharedMeshBundle);
	CSharedMeshBundle() { };
	virtual ~CSharedMeshBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_S_MESH(rid) dingus::CSharedMeshBundle::getInstance().getResourceById(rid)


#endif
