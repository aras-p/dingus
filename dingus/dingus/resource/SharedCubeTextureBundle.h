// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SHARED_CUBE_TEXTURE_BUNDLE_H
#define __SHARED_CUBE_TEXTURE_BUNDLE_H

#include "SharedResourceBundle.h"
#include "CubeTextureCreator.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSharedCubeTextureBundle :	public CSharedResourceBundle<CD3DCubeTexture,ICubeTextureCreator::TSharedPtr>,
								public CSingleton<CSharedCubeTextureBundle>,
								public IDeviceResource
{
public:
	void registerCubeTexture( const CResourceId& id, ICubeTextureCreator& creator );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CD3DCubeTexture& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSharedCubeTextureBundle);
	CSharedCubeTextureBundle() { };
	virtual ~CSharedCubeTextureBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_S_CUBETEX(rid) dingus::CSharedCubeTextureBundle::getInstance().getResourceById(rid)


#endif
