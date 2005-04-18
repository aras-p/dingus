// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DYNAMIC_IB_MANAGER_H
#define __DYNAMIC_IB_MANAGER_H

#include "ManagedBuffer.h"
#include "BufferChunk.h"
#include "../../resource/DeviceResource.h"
#include "../../utils/Singleton.h"

namespace dingus {


class CDynamicIBManager : public CManagedBuffer<TIBChunk, CD3DIndexBuffer>,
						public IDeviceResource,
						public CSingleton<CDynamicIBManager>
{
public:
	enum { DEFAULT_CAPACITY = 1*1024*1024 }; // 1 megabyte
	static const DWORD		IB_USAGE;
	static const D3DPOOL	IB_POOL;

public:
	static void initialize( int capacityBytes ) {
		CDynamicIBManager* ibmgr = new CDynamicIBManager( capacityBytes );
		assert( ibmgr );
		assignInstance( *ibmgr );
	}

public:
	virtual ~CDynamicIBManager();
	
	// IManagedResource
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

protected:
	// interface for CManagedBuffer
	virtual byte* lockBuffer( int byteStart, int byteCount );

	CD3DIndexBuffer* allocateBuffer( int capacityBytes );

private:
	IDirect3DIndexBuffer9* createBuffer( int capacityBytes );

private:
	// CSingleton
	CDynamicIBManager( int capacityBytes );
	static CDynamicIBManager* createInstance() {
		ASSERT_FAIL_MSG( "must be initialized first" );
		return NULL;
	}
	static void deleteInstance( CDynamicIBManager& instance ) { delete &instance; }
	friend CSingleton<CDynamicIBManager>;
};


}; // namespace

#endif
