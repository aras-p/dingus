// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SHARED_TEXTURE_BUNDLE_H
#define __SHARED_TEXTURE_BUNDLE_H

#include "SharedResourceBundle.h"
#include "TextureCreator.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


class CSharedTextureBundle :	public CSharedResourceBundle<CD3DTexture,ITextureCreator::TSharedPtr>,
								public CSingleton<CSharedTextureBundle>,
								public IDeviceResource
{
public:
	void registerTexture( CResourceId const& id, ITextureCreator& creator );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CD3DTexture& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CSharedTextureBundle);
	CSharedTextureBundle() { };
	virtual ~CSharedTextureBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_S_TEX(rid) dingus::CSharedTextureBundle::getInstance().getResourceById(rid)


#endif
