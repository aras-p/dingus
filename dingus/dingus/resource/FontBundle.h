// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FONT_BUNDLE_H
#define __FONT_BUNDLE_H

#include "../gfx/gui/UIFont.h"
#include "StorageResourceBundle.h"
#include "../utils/Singleton.h"


namespace dingus {


class CFontBundle : public CStorageResourceBundle<CGUIFont>, public CSingleton<CFontBundle> {
protected:
	virtual CGUIFont* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CGUIFont& resource ) { delete &resource; }
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CFontBundle);
	CFontBundle();
	virtual ~CFontBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_FONT(rid) dingus::CFontBundle::getInstance().getResourceById(rid)


#endif
