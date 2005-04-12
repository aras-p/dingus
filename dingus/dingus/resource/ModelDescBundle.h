// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MODEL_DESC_BUNDLE_H
#define __MODEL_DESC_BUNDLE_H

#include "StorageResourceBundle.h"
#include "../gfx/ModelDesc.h"
#include "../utils/Singleton.h"

namespace dingus {


class CModelDescBundle : public CStorageResourceBundle<CModelDesc>,
						public CSingleton<CModelDescBundle>
{
protected:
	virtual CModelDesc* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CModelDesc& resource ) { delete &resource; }
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CModelDescBundle);
	CModelDescBundle();
	virtual ~CModelDescBundle() { clear(); };
	
	/// @return false on not found
	bool loadModelDesc( const CResourceId& id, const CResourceId& fullName, CModelDesc& desc ) const;
};


}; // namespace


/// Shortcut macro
#define RGET_MDESC(rid) dingus::CModelDescBundle::getInstance().getResourceById(rid)
/// Shortcut macro
#define RTRYGET_MDESC(rid) dingus::CModelDescBundle::getInstance().trygetResourceById(rid)


#endif
