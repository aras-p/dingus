// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DEV_RELOADABLE_RESOURCE_H
#define __DEV_RELOADABLE_RESOURCE_H

#include "ReloadableBundle.h"
#include "../utils/Singleton.h"


namespace dingus {


/**
 *  D3D device dependant resource.
 */
class IDeviceResource {
public:
	virtual ~IDeviceResource() = 0 {};

	virtual void createResource() = 0;
	virtual void activateResource() = 0;
	virtual void passivateResource() = 0;
	virtual void deleteResource() = 0;
};


class IDeviceReloadableBundle : public IDeviceResource, public IReloadableBundle {
public:
	virtual void reload() {
		passivateResource();
		deleteResource();
		createResource();
		activateResource();
	}
};


class CDeviceResourceManager : public IDeviceResource,
							public CNotifier<IDeviceResource>,
							public CSingleton<CDeviceResourceManager>
{
public:
	// IDeviceResource
	virtual void createResource() {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it )
			(*it)->createResource();
	}
	virtual void activateResource() {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it )
			(*it)->activateResource();
	}
	virtual void passivateResource() {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it )
			(*it)->passivateResource();
	}
	virtual void deleteResource() {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it )
			(*it)->deleteResource();
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CDeviceResourceManager);
	CDeviceResourceManager() { };
	virtual ~CDeviceResourceManager() { };
};


}; // namespace

#endif
