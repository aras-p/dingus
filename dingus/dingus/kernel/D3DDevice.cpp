// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "D3DDevice.h"
#include "../utils/Errors.h"

using namespace dingus;

CD3DDevice CD3DDevice::mSingleInstance;

// define for testing only
//#define DISABLE_FILTERING


// --------------------------------------------------------------------------

CD3DDevice::CD3DDevice()
:	mDevice(NULL),
	mDeviceWindow(NULL),
	mBackBuffer(NULL),
	mMainZStencil(NULL)
{
	resetCachedState();
}

void CD3DDevice::resetCachedState()
{
	mStateManager.reset();

	int i;
	for( i = 0; i < MRT_COUNT; ++i )
		mActiveRT[i] = (IDirect3DSurface9*)-1;
	mActiveZS = (IDirect3DSurface9*)-1;
	for( i = 0; i < VSTREAM_COUNT; ++i ) {
		mActiveVB[i] = (IDirect3DVertexBuffer9*)-1;
		mActiveVBOffset[i] = (unsigned int)-1;
		mActiveVBStride[i] = (unsigned int)-1;
	}
	mActiveIB = (CD3DIndexBuffer*)-1;
	mActiveDeclaration = (CD3DVertexDecl*)-1;
	mActiveFVF = (DWORD)-1;
}

void CD3DDevice::activateDevice( HWND deviceWindow )
{
	assert( !mMainZStencil );
	assert( !mBackBuffer );

	mDeviceWindow = deviceWindow;

	resetCachedState();

	// backbuffer
	mDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &mBackBuffer );
	assert( mBackBuffer );
	mBackBuffer->GetDesc( &mBackBufferDesc );
	mBackBufferAspect = (float)getBackBufferWidth() / (float)getBackBufferHeight();
	mActiveRT[0] = mBackBuffer;

	// Z/stencil
	mDevice->GetDepthStencilSurface( &mMainZStencil );
	if( mMainZStencil ) {
		assert( mMainZStencil );
		mMainZStencil->GetDesc( &mMainZStencilDesc );
		mActiveZS = mMainZStencil;
	} else {
		memset( &mMainZStencilDesc, 0, sizeof(mMainZStencilDesc) );
	}

	mStats.reset();
}

void CD3DDevice::passivateDevice()
{
	int i;

	mDeviceWindow = NULL;

	// set default rendertargets
	setDefaultRenderTarget();
	for( i = 1; i < mCaps.getMRTCount(); ++i )
		setRenderTarget( NULL, i );
	setDefaultZStencil();

	// set null streams
	setIndexBuffer( NULL );
	for( i = 0; i < VSTREAM_COUNT; ++i )
		setVertexBuffer( 0, NULL, 0, 0 );
	setDeclarationFVF( D3DFVF_XYZ );

	// release backbuffer/zstencil
	if( mBackBuffer ) {
		mBackBuffer->Release();
		mBackBuffer = NULL;
	}
	if( mMainZStencil ) {
		mMainZStencil->Release();
		mMainZStencil = NULL;
	}
}

void CD3DDevice::setDevice( IDirect3DDevice9* dx, CD3DDeviceCaps::eVertexProcessing vertexProc )
{
	mCaps = CD3DDeviceCaps();

	mDevice = dx;
	if( !mDevice ) {
		assert( !mBackBuffer );
		assert( !mMainZStencil );
		return;
	}
	assert( mDevice );

	IDirect3D9* d3d = 0;
	D3DCAPS9 caps;
	D3DDEVICE_CREATION_PARAMETERS createParams;
	dx->GetDirect3D( &d3d );
	mDevice->GetDeviceCaps( &caps );
	mDevice->GetCreationParameters( &createParams );

	mCaps.setData( *d3d, createParams.AdapterOrdinal, createParams.DeviceType, caps );
	mCaps.setVertexProcessing( vertexProc );

	assert( !mBackBuffer );
	assert( !mMainZStencil );
}


// --------------------------------------------------------------------------

void CD3DDevice::setRenderTarget( CD3DSurface* rt, int index )
{
	internalSetRenderTarget( rt ? rt->getObject() : NULL, index );
}

void CD3DDevice::setDefaultRenderTarget()
{
	internalSetRenderTarget( mBackBuffer, 0 );
}

void CD3DDevice::internalSetRenderTarget( IDirect3DSurface9* rt, int index )
{
	assert( mDevice );

	// #MRT check
	if( index >= mCaps.getMRTCount() ) {
		// debug complaint
		ASSERT_FAIL_MSG( "rendertarget index larger than number of MRTs supported!" );
		return;
	}

	// redundant set check
#ifndef DISABLE_FILTERING
	if( mActiveRT[index] == rt ) {
		++mStats.filtered.renderTarget;
		return;
	}
#endif

	// set RT
	HRESULT hr = mDevice->SetRenderTarget( index, rt );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set render target" );

	mActiveRT[index] = rt;
	++mStats.changes.renderTarget;
}

void CD3DDevice::setZStencil( CD3DSurface* zs )
{
	internalSetZStencil( zs ? zs->getObject() : NULL );
}

void CD3DDevice::setDefaultZStencil()
{
	internalSetZStencil( mMainZStencil );
}

void CD3DDevice::internalSetZStencil( IDirect3DSurface9* zs )
{
	assert( mDevice );

	// redundant set check
#ifndef DISABLE_FILTERING
	if( mActiveZS == zs ) {
		++mStats.filtered.zStencil;
		return;
	}
#endif

	// set RT
	HRESULT hr = mDevice->SetDepthStencilSurface( zs );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set z/stencil" );

	mActiveZS = zs;
	++mStats.changes.zStencil;
}

void CD3DDevice::clearTargets( bool clearRT, bool clearZ, bool clearStencil, D3DCOLOR color, float z, DWORD stencil )
{
	assert( mDevice );

	DWORD clrFlags = 0;
	if( clearRT ) clrFlags |= D3DCLEAR_TARGET;
	if( clearZ ) clrFlags |= D3DCLEAR_ZBUFFER;
	if( clearStencil ) clrFlags |= D3DCLEAR_STENCIL;

	HRESULT hr = mDevice->Clear( 0, NULL, clrFlags, color, z, stencil );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to clear render target" );
}


// --------------------------------------------------------------------------

void CD3DDevice::sceneBegin()
{
	assert( mDevice );
	HRESULT hr = mDevice->BeginScene();
	if( FAILED(hr) )
		THROW_DXERROR( hr, "failed to begin scene" );
}

void CD3DDevice::sceneEnd()
{
	assert( mDevice );
	HRESULT hr = mDevice->EndScene();
	if( FAILED(hr) )
		THROW_DXERROR( hr, "failed to end scene" );
}

// --------------------------------------------------------------------------

void CD3DDevice::setIndexBuffer( CD3DIndexBuffer* ib )
{
	assert( mDevice );
	
	// redundant set check
#ifndef DISABLE_FILTERING
	if( mActiveIB == ib ) {
		++mStats.filtered.ibuffer;
		return;
	}
#endif

	// set
	HRESULT hr = mDevice->SetIndices( ib ? ib->getObject() : NULL );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set indices" );

	mActiveIB = ib;
	++mStats.changes.ibuffer;
}

void CD3DDevice::setVertexBuffer( int stream,  CD3DVertexBuffer* vb, unsigned int offset, unsigned int stride )
{
	assert( mDevice );
	assert( stream >= 0 && stream < VSTREAM_COUNT );
	
	// redundant set check
	IDirect3DVertexBuffer9* vb9 = vb ? vb->getObject() : NULL;
#ifndef DISABLE_FILTERING
	if( mActiveVB[stream]==vb9 && mActiveVBOffset[stream]==offset && mActiveVBStride[stream]==stride ) {
		++mStats.filtered.vbuffer;
		return;
	}
#endif

	// set
	HRESULT hr = mDevice->SetStreamSource( stream, vb9, offset, stride );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set vertex buffer" );

	mActiveVB[stream] = vb9;
	mActiveVBOffset[stream] = offset;
	mActiveVBStride[stream] = stride;
	++mStats.changes.vbuffer;
}

void CD3DDevice::setDeclaration( CD3DVertexDecl& decl )
{
	assert( mDevice );
	assert( &decl );
	
	// redundant set check
#ifndef DISABLE_FILTERING
	if( mActiveDeclaration == &decl ) {
		++mStats.filtered.declarations;
		return;
	}
#endif

	// set
	HRESULT hr = mDevice->SetVertexDeclaration( &decl ? decl.getObject() : NULL );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set declaration" );

	mActiveDeclaration = &decl;
	mActiveFVF = 0; // FVF and declaration are coupled!
	++mStats.changes.declarations;
}

void CD3DDevice::setDeclarationFVF( DWORD fvf )
{
	assert( mDevice );
	assert( fvf );
	
	// redundant set check
#ifndef DISABLE_FILTERING
	if( mActiveFVF == fvf ) {
		++mStats.filtered.declarations;
		return;
	}
#endif

	// set
	HRESULT hr = mDevice->SetFVF( fvf );
	if( FAILED( hr ) )
		THROW_DXERROR( hr, "failed to set declaration fvf" );

	mActiveFVF = fvf; 
	mActiveDeclaration = NULL; // FVF and declaration are coupled!
	++mStats.changes.declarations;
}
