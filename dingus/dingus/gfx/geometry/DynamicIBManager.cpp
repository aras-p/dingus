// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "DynamicIBManager.h"
#include "../../kernel/D3DDevice.h"
#include "../../utils/Errors.h"

using namespace dingus;



const DWORD CDynamicIBManager::IB_USAGE = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
const D3DPOOL CDynamicIBManager::IB_POOL = D3DPOOL_DEFAULT;


CDynamicIBManager::CDynamicIBManager( unsigned int capacityBytes )
:	CManagedBuffer<TIBChunk,CD3DIndexBuffer>(capacityBytes)
{
	mBuffer = new CD3DIndexBuffer( NULL );
}

CDynamicIBManager::~CDynamicIBManager()
{
	delete mBuffer;
}

CD3DIndexBuffer* CDynamicIBManager::allocateBuffer( unsigned int capacityBytes )
{
	return new CD3DIndexBuffer( createBuffer( capacityBytes ) );
}

IDirect3DIndexBuffer9* CDynamicIBManager::createBuffer( unsigned int capacityBytes )
{
	IDirect3DIndexBuffer9* ib = NULL;
	HRESULT hres = CD3DDevice::getInstance().getDevice().CreateIndexBuffer(
		capacityBytes,
		IB_USAGE,
		D3DFMT_INDEX16, // TBD: 32 bit dynamic IB!
		IB_POOL,
		&ib, NULL );
	if( FAILED( hres ) ) {
		THROW_DXERROR( hres, "failed to create index buffer" );
	}
	return ib;
}

byte* CDynamicIBManager::lockBuffer( unsigned int byteStart, unsigned int byteCount )
{
	if( byteCount == 0 )
		return NULL;

	// append data
	DWORD lockFlag = D3DLOCK_NOOVERWRITE;
	
	// ib must be discarded at first
	if( byteStart == 0 )
		lockFlag = D3DLOCK_DISCARD;

	byte* data = NULL;
	HRESULT hres = mBuffer->getObject()->Lock(
		byteStart,
		byteCount,
		reinterpret_cast<void**>( &data ),
		lockFlag );
	if( FAILED( hres ) ) {
		THROW_DXERROR( hres, "failed to lock index buffer" );
	}

	return data;
}


void CDynamicIBManager::createResource()
{
}

void CDynamicIBManager::activateResource()
{
	if( !mBuffer->isNull() )
		return;
	mBuffer->setObject( createBuffer( getCapacityBytes() ) );
	assert( !mBuffer->isNull() );
}

void CDynamicIBManager::passivateResource()
{
	assert( !mBuffer->isNull() );
	discard();
	mBuffer->getObject()->Release();
	mBuffer->setObject( NULL );
}

void CDynamicIBManager::deleteResource()
{
}
