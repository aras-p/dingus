// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SKELINFO_BUNDLE_H
#define __SKELINFO_BUNDLE_H

#include "../gfx/skeleton/SkeletonInfo.h"
#include "StorageResourceBundle.h"
#include "../utils/Singleton.h"


namespace dingus {


class CSkeletonInfoBundle : public CStorageResourceBundle<CSkeletonInfo>, public CSingleton<CSkeletonInfoBundle>
{
protected:
	virtual CSkeletonInfo* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CSkeletonInfo& resource ) { delete &resource; }
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CSkeletonInfoBundle);
	CSkeletonInfoBundle();
	virtual ~CSkeletonInfoBundle() { clear(); };
};

}; // namespace


/// Shortcut macro
#define RGET_SKEL(rid) dingus::CSkeletonInfoBundle::getInstance().getResourceById(rid)


#endif
