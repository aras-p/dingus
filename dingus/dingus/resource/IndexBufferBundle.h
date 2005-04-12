// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __IB_BUNLDE_H
#define __IB_BUNLDE_H

#include "SharedResourceBundle.h"
#include "../kernel/Proxies.h"
#include "DeviceResource.h"
#include "../utils/Singleton.h"

namespace dingus {


/**
 *  Index buffer filler.
 *
 *  This object is used to initialize static index buffer.
 */
class IIndexBufferFiller : public CRefCounted {
public:
	typedef DingusSmartPtr<IIndexBufferFiller> TSharedPtr;
public:
	virtual ~IIndexBufferFiller() = 0 { }
	virtual void fillIB( CD3DIndexBuffer& ib, int elements, D3DFORMAT format ) = 0;
};


// Index buffer descriptor. Used internally by CIndexBufferBundle.
struct SIndexBufferDesc {
	int			elements;
	D3DFORMAT	format;
	IIndexBufferFiller::TSharedPtr	filler;
};


/**
 *  Bundle for shared static index buffers.
 */
class CIndexBufferBundle :	public CSharedResourceBundle<CD3DIndexBuffer,SIndexBufferDesc>,
							public CSingleton<CIndexBufferBundle>,
							public IDeviceResource
{
public:
	/**
	 *  Registers IB.
	 *  @note Filler object must exist for the whole lifetime of bundle.
	 */
	void registerIB(
		CResourceId const& id,
		int elements, D3DFORMAT format, IIndexBufferFiller& filler );

	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	virtual void deleteResource( CD3DIndexBuffer& resource ) {
		if( resource.getObject() )
			resource.getObject()->Release();
		delete &resource;
	}

private:
	IMPLEMENT_SIMPLE_SINGLETON(CIndexBufferBundle);
	CIndexBufferBundle() { };
	virtual ~CIndexBufferBundle() { clear(); };
	
	IDirect3DIndexBuffer9* createIB( const SIndexBufferDesc&d ) const;
};

}; // namespace


/// Shortcut macro
#define RGET_IB(rid) dingus::CIndexBufferBundle::getInstance().getResourceById(rid)


#endif
