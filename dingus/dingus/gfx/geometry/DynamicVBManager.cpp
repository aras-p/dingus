// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "DynamicVBManager.h"
#include "../../kernel/D3DDevice.h"
#include "../../utils/Errors.h"

using namespace dingus;



const DWORD CDynamicVBManager::VB_USAGE = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
const D3DPOOL CDynamicVBManager::VB_POOL = D3DPOOL_DEFAULT;

CDynamicVBManager::CDynamicVBManager( int capacityBytes )
:	CManagedBuffer<CVBChunk,CD3DVertexBuffer>(capacityBytes)
{
	mBuffer = new CD3DVertexBuffer( NULL );
}

CDynamicVBManager::~CDynamicVBManager()
{
	delete mBuffer;
}

CD3DVertexBuffer* CDynamicVBManager::allocateBuffer( int capacityBytes )
{
	return new CD3DVertexBuffer( createBuffer( capacityBytes ) );
}

IDirect3DVertexBuffer9* CDynamicVBManager::createBuffer( int capacityBytes )
{
	IDirect3DVertexBuffer9* vb = NULL;
	HRESULT hres = CD3DDevice::getInstance().getDevice().CreateVertexBuffer(
		capacityBytes,
		VB_USAGE,
		0,
		VB_POOL,
		&vb, NULL );
	if( FAILED( hres ) ) {
		THROW_DXERROR( hres, "failed to create vertex buffer" );
	}
	return vb;
}

byte* CDynamicVBManager::lockBuffer( int byteStart, int byteCount )
{
	if( byteCount == 0 )
		return NULL;

	// append data
	DWORD lockFlag = D3DLOCK_NOOVERWRITE;
	
	// vb must be discarded at first
	if( byteStart == 0 )
		lockFlag = D3DLOCK_DISCARD;

	byte* data = NULL;
	HRESULT hres = mBuffer->getObject()->Lock(
		byteStart, 
		byteCount,
		reinterpret_cast<void**>( &data ),
		lockFlag );
	if( FAILED( hres ) ) {
		THROW_DXERROR( hres, "failed to lock vertex buffer" );
	}

	return data;
}


void CDynamicVBManager::createResource()
{
}

void CDynamicVBManager::activateResource()
{
	// first creation can happen only here
	//if( !mBuffer )
	//	mBuffer = new CD3DVertexBuffer( NULL );

	if( !mBuffer->isNull() )
		return;
	mBuffer->setObject( createBuffer( getCapacityBytes() ) );
	assert( !mBuffer->isNull() );
}

void CDynamicVBManager::passivateResource()
{
	assert( !mBuffer->isNull() );
	discard();
	mBuffer->getObject()->Release();
	mBuffer->setObject( NULL );
}

void CDynamicVBManager::deleteResource()
{
}
