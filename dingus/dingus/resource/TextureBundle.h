// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TEXTURE_BUNDLE_H
#define __TEXTURE_BUNDLE_H

#include "StorageResourceBundle.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CTextureBundle :	public CStorageResourceBundle<CD3DTexture>,
						public CSingleton<CTextureBundle>,
						public IDeviceReloadableBundle
{
public:
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual CD3DTexture* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CD3DTexture& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CTextureBundle);
	CTextureBundle();
	virtual ~CTextureBundle() { clear(); };
	
	IDirect3DTexture9* loadTexture( const CResourceId& id, const CResourceId& fullName ) const;
};


}; // namespace


/// Shortcut macro
#define RGET_TEX(rid) dingus::CTextureBundle::getInstance().getResourceById(rid)


#endif
