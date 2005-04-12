// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SHARED_VOLUME_BUNDLE_H
#define __SHARED_VOLUME_BUNDLE_H

#include "SharedResourceBundle.h"
#include "VolumeCreator.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSharedVolumeBundle :	public CSharedResourceBundle<CD3DVolumeTexture,IVolumeCreator::TSharedPtr>,
								public CSingleton<CSharedVolumeBundle>,
								public IDeviceResource
{
public:
	void registerVolume( CResourceId const& id, IVolumeCreator& creator );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CD3DVolumeTexture& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSharedVolumeBundle);
	CSharedVolumeBundle() { };
	virtual ~CSharedVolumeBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_S_VOLTEX(rid) dingus::CSharedVolumeBundle::getInstance().getResourceById(rid)


#endif
