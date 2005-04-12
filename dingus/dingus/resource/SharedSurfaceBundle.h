// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#pragma warning(disable:4786)
#ifndef __SHARED_SURFACE_BUNDLE_H
#define __SHARED_SURFACE_BUNDLE_H

#include "SharedResourceBundle.h"
#include "SurfaceCreator.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSharedSurfaceBundle :public CSharedResourceBundle<CD3DSurface,ISurfaceCreator::TSharedPtr>,
							public CSingleton<CSharedSurfaceBundle>,
							public IDeviceResource
{
public:
	void registerSurface( CResourceId const& id, ISurfaceCreator& creator );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CD3DSurface& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSharedSurfaceBundle);
	CSharedSurfaceBundle() { };
	virtual ~CSharedSurfaceBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_S_SURF(rid) dingus::CSharedSurfaceBundle::getInstance().getResourceById(rid)


#endif
