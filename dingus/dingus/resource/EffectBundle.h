// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __EFFECT_BUNDLE_H
#define __EFFECT_BUNDLE_H

#include "StorageResourceBundle.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {



/**
 *  D3DX Effects bundle.
 *
 *  Enables conditional compilation of effects via macros (name-value pairs),
 *  see setMacro() and removeMacro().
 */
class CEffectBundle :	public CStorageResourceBundle<CD3DXEffect>,
						public CSingleton<CEffectBundle>,
						public IDeviceReloadableBundle
{
public:
	bool	isOptimizingShaders() const { return mOptimizeShaders; }
	void	setOptimizeShaders( bool opt ) { mOptimizeShaders = opt; }
	bool	isUsingStateManager() const { return mUseStateManager; }
	void	setUsingStateManager( bool stmgr ) { mUseStateManager = stmgr; }
	const std::string& getLastErrors() const { return mLastErrors; }

	/**
	 *  Adds macro (or replaces same-named one).
	 *  After changing a bunch of macros, call reload() to actually reload effects.
	 *  NOTE: name/value strings are not copied, so look at their lifetime!
	 */
	void setMacro( const char* name, const char* value );
	/**
	 *  Removes macro.
	 *  After changing a bunch of macros, call reload() to actually reload effects.
	 */
	void removeMacro( const char* name );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual CD3DXEffect* loadResourceById( const CResourceId& id, const CResourceId& fullName );
	virtual void deleteResource( CD3DXEffect& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CEffectBundle);
	CEffectBundle();
	virtual ~CEffectBundle() { clear(); };
	
	ID3DXEffect* loadEffect( const CResourceId& id, const CResourceId& fullName ) const;

	/// @return index into macro array, or -1 if not found.
	int		findMacro( const char* name ) const;
	

	typedef std::vector<D3DXMACRO>	TMacroVector;

private:
	ID3DXEffectPool*		mSharedPool;

	bool	mUseStateManager;
	bool	mOptimizeShaders;
	mutable std::string		mLastErrors;

	TMacroVector			mMacros;
};


}; // namespace


/// Shortcut macro
#define RGET_FX(rid) dingus::CEffectBundle::getInstance().getResourceById(rid)


#endif
