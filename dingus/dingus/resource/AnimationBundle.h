// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIMATION_BUNDLE_H
#define __ANIMATION_BUNDLE_H

#include "../animator/AnimationBunch.h"
#include "StorageResourceBundle.h"
#include "../utils/Singleton.h"

namespace dingus {


class CAnimationBundle : public CStorageResourceBundle<CAnimationBunch>,
						public CSingleton<CAnimationBundle>
{
protected:
	virtual CAnimationBunch* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CAnimationBunch& resource ) { delete &resource; }
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CAnimationBundle);
	CAnimationBundle();
	virtual ~CAnimationBundle() { clear(); };

	void readSampledData( CAnimationBunch& bunch, FILE* f, int loop, int curves, int groups ) const;
};

}; // namespace


/// Shortcut macro
#define RGET_ANIM(rid) dingus::CAnimationBundle::getInstance().getResourceById(rid)


#endif
