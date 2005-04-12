// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __CUBE_TEXTURE_BUNDLE_H
#define __CUBE_TEXTURE_BUNDLE_H

#include "StorageResourceBundle.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CCubeTextureBundle :	public CStorageResourceBundle<CD3DCubeTexture>,
						public CSingleton<CCubeTextureBundle>,
						public IDeviceReloadableBundle
{
public:
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual CD3DCubeTexture* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CD3DCubeTexture& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CCubeTextureBundle);
	CCubeTextureBundle();
	virtual ~CCubeTextureBundle() { clear(); };
	
	IDirect3DCubeTexture9* loadTexture( const CResourceId& id, const CResourceId& fullName ) const;
};


}; // namespace


/// Shortcut macro
#define RGET_CUBETEX(rid) dingus::CCubeTextureBundle::getInstance().getResourceById(rid)


#endif
