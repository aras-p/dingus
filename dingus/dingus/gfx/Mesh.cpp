// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "Mesh.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


CMesh::CMesh()
:	mVertexCount(0), mIndexCount(0),
	mVertexFormat(0), mVertexStride(0),
	mIndexStride(0), mVertexDecl(0),
	mVB(0), mIB(0),
	mVBLock(NOLOCK), mIBLock(NOLOCK)
{
}

CMesh::~CMesh()
{
	ASSERT_MSG( mVBLock==NOLOCK, "VB still locked" );
	ASSERT_MSG( mIBLock==NOLOCK, "IB still locked" );
	ASSERT_MSG( !isCreated(), "mesh not cleaned up" );
}


void CMesh::createResource( int vertCount, int idxCount,
		const CVertexFormat& vertFormat, int indexStride, CD3DVertexDecl& vertDecl,
		eBufferType bufferType )
{
	ASSERT_MSG( !isCreated(), "mesh not cleaned up" );
	ASSERT_MSG( vertCount > 0, "bad vertex count" );
	ASSERT_MSG( idxCount > 0, "bad index count" );
	ASSERT_MSG( indexStride==2 || indexStride==4, "bad index stride" );
	mBufferType = bufferType;
	mVertexCount = vertCount;
	mIndexCount = idxCount;
	mVertexFormat = vertFormat;
	mVertexStride = vertFormat.calcVertexSize();
	ASSERT_MSG( mVertexStride > 0, "bad vertex stride" );
	mIndexStride = indexStride;
	mVertexDecl = &vertDecl;
	ASSERT_MSG( mVertexDecl, "null vertex decl" );

	// create buffers
	CD3DDevice& dx = CD3DDevice::getInstance();
	HRESULT hr;

	DWORD usage = 0;
	D3DPOOL bufpool = (mBufferType == BUF_STATIC) ? D3DPOOL_MANAGED : D3DPOOL_SYSTEMMEM;
	CD3DDeviceCaps::eVertexProcessing vproc = dx.getCaps().getVertexProcessing();
	if( (vproc == CD3DDeviceCaps::VP_SW) ||
		(vproc == CD3DDeviceCaps::VP_MIXED && vertFormat.getSkinDataMode() != CVertexFormat::FLT3_NONE) ||
		(vproc == CD3DDeviceCaps::VP_MIXED && vertCount==4 && idxCount==6) ) // HACK
	{
		usage |= D3DUSAGE_SOFTWAREPROCESSING;
		bufpool = D3DPOOL_SYSTEMMEM;
	}

	IDirect3DVertexBuffer9* vb = 0;
	hr = dx.getDevice().CreateVertexBuffer( mVertexCount * mVertexStride, usage, 0, bufpool, &vb, 0 );
	assert( SUCCEEDED(hr) && vb );
	mVB.setObject( vb );
	IDirect3DIndexBuffer9* ib = 0;
	hr = dx.getDevice().CreateIndexBuffer( mIndexCount * mIndexStride, usage, mIndexStride==2?D3DFMT_INDEX16:D3DFMT_INDEX32, bufpool, &ib, 0 );
	assert( SUCCEEDED(hr) && ib );
	mIB.setObject( ib );

	// clear groups
	mGroups.clear();
}

void CMesh::deleteResource()
{
	ASSERT_MSG( isCreated(), "mesh not created" );
	ASSERT_MSG( mVBLock==NOLOCK, "VB still locked" );
	ASSERT_MSG( mIBLock==NOLOCK, "IB still locked" );

	mVB.getObject()->Release();
	mVB.setObject( 0 );
	mIB.getObject()->Release();
	mIB.setObject( 0 );

	// clear groups
	mGroups.clear();
}


const void* CMesh::lockVBRead() const
{
	ASSERT_MSG( mVBLock==NOLOCK, "VB still locked" );
	assert( !mVB.isNull() );
	void* data = 0;
	mVB.getObject()->Lock( 0, 0, &data, D3DLOCK_READONLY );
	assert( data );
	mVBLock = LOCK_READ;
	return data;
}
void CMesh::unlockVBRead() const
{
	ASSERT_MSG( mVBLock==LOCK_READ, "VB not locked for reading" );
	assert( !mVB.isNull() );
	mVB.getObject()->Unlock();
	mVBLock = NOLOCK;
}
void* CMesh::lockVBWrite()
{
	ASSERT_MSG( mVBLock==NOLOCK, "VB still locked" );
	assert( !mVB.isNull() );
	void* data = 0;
	mVB.getObject()->Lock( 0, 0, &data, 0 );
	assert( data );
	mVBLock = LOCK_WRITE;
	return data;
}
void CMesh::unlockVBWrite()
{
	ASSERT_MSG( mVBLock==LOCK_WRITE, "VB not locked for writing" );
	assert( !mVB.isNull() );
	mVB.getObject()->Unlock();
	mVBLock = NOLOCK;
}


const void* CMesh::lockIBRead() const
{
	ASSERT_MSG( mIBLock==NOLOCK, "IB still locked" );
	assert( !mIB.isNull() );
	void* data = 0;
	mIB.getObject()->Lock( 0, 0, &data, D3DLOCK_READONLY );
	assert( data );
	mIBLock = LOCK_READ;
	return data;
}
void CMesh::unlockIBRead() const
{
	ASSERT_MSG( mIBLock==LOCK_READ, "IB not locked for reading" );
	assert( !mIB.isNull() );
	mIB.getObject()->Unlock();
	mIBLock = NOLOCK;
}
void* CMesh::lockIBWrite()
{
	ASSERT_MSG( mIBLock==NOLOCK, "IB still locked" );
	assert( !mIB.isNull() );
	void* data = 0;
	mIB.getObject()->Lock( 0, 0, &data, 0 );
	assert( data );
	mIBLock = LOCK_WRITE;
	return data;
}
void CMesh::unlockIBWrite()
{
	ASSERT_MSG( mIBLock==LOCK_WRITE, "IB not locked for writing" );
	assert( !mIB.isNull() );
	mIB.getObject()->Unlock();
	mIBLock = NOLOCK;
}

void CMesh::getTriIndices( const void* lockedIB, int tri, int& idx0, int& idx1, int& idx2 ) const
{
	ASSERT_MSG( mIBLock==LOCK_READ, "IB not locked for reading" );
	assert( lockedIB );
	assert( tri >= 0 && tri < mIndexCount/3 );
	if( mIndexStride==2 ) {
		// 2 byte indices
		const unsigned short* ib = (const unsigned short*)lockedIB;
		ib += tri*3;
		idx0 = ib[0];
		idx1 = ib[1];
		idx2 = ib[2];
	} else {
		// 4 byte indices
		const int* ib = (const int*)lockedIB;
		ib += tri*3;
		idx0 = ib[0];
		idx1 = ib[1];
		idx2 = ib[2];
	}
}


void CMesh::computeAABBs()
{
	mTotalAABB.setNull();
	if( !mVertexFormat.hasPosition() )
		return;
	const char* vb = reinterpret_cast<const char*>( lockVBRead() );
	size_t nGroups = mGroups.size();
	for( size_t i = 0; i < nGroups; ++i ) {
		CGroup& g = mGroups[i];
		g.getAABB().setNull();
		for( int v = 0; v < g.getVertexCount(); ++v ) {
			const char* vtx = vb + mVertexStride * ( v + g.getFirstVertex() );
			const SVector3& pos = *reinterpret_cast<const SVector3*>(vtx);
			g.getAABB().extend( pos );
		}
		mTotalAABB.extend( g.getAABB() );
	}
	unlockVBRead();

	// kind of HACK: if we're skinned, inflate total AABB by some amount
	if( mVertexFormat.getSkinDataMode() != CVertexFormat::FLT3_NONE ) {
		SVector3 aabbSize = mTotalAABB.getMax() - mTotalAABB.getMin();
		if( aabbSize.x < aabbSize.y ) aabbSize.x = aabbSize.y;
		if( aabbSize.z < aabbSize.y ) aabbSize.z = aabbSize.y;
		mTotalAABB.getMin() -= aabbSize;
		mTotalAABB.getMax() += aabbSize;
	}
}

ID3DXMesh* CMesh::createD3DXMesh() const
{
	HRESULT hr;
	ID3DXMesh* dxMesh = 0;

	DWORD meshOpts = D3DXMESH_MANAGED;
	if( getIndexStride() == 4 )
		meshOpts |= D3DXMESH_32BIT;


	// get declaration
	D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
	UINT numElements;
	getVertexDecl().getObject()->GetDeclaration( decl, &numElements );
	// create mesh
	hr = D3DXCreateMesh( getIndexCount()/3, getVertexCount(), meshOpts, decl, &CD3DDevice::getInstance().getDevice(), &dxMesh );
	if( FAILED(hr) )
		return NULL;

	// copy VB
	{
		const void* srcVB = lockVBRead();
		void* dxVB = 0;
		hr = dxMesh->LockVertexBuffer( 0, &dxVB );
		if( FAILED(hr) ) {
			dxMesh->Release();
			return NULL;
		}
		memcpy( dxVB, srcVB, getVertexCount() * getVertexStride() );
		hr = dxMesh->UnlockVertexBuffer();
		unlockVBRead();
	}
	// copy IB
	{
		const void* srcIB = lockIBRead();
		void* dxIB = 0;
		hr = dxMesh->LockIndexBuffer( 0, &dxIB );
		if( FAILED(hr) ) {
			dxMesh->Release();
			return NULL;
		}
		memcpy( dxIB, srcIB, getIndexCount() * getIndexStride() );
		hr = dxMesh->UnlockIndexBuffer();
		unlockIBRead();
	}
	// copy groups
	{
		int ngroups = getGroupCount();
		D3DXATTRIBUTERANGE* attrs = new D3DXATTRIBUTERANGE[ngroups];
		DWORD* attrBuf = 0;
		hr = dxMesh->LockAttributeBuffer( 0, &attrBuf );
		if( FAILED(hr) ) {
			dxMesh->Release();
			return NULL;
		}
		for( int g = 0; g < ngroups; ++g ) {
			attrs[g].AttribId = g;
			const CMesh::CGroup& group = getGroup(g);
			attrs[g].VertexStart = group.getFirstVertex();
			attrs[g].VertexCount = group.getVertexCount();
			attrs[g].FaceStart = group.getFirstPrim();
			attrs[g].FaceCount = group.getPrimCount();
			for( int f = 0; f < group.getPrimCount(); ++f )
				*attrBuf++ = g;
		}
		dxMesh->UnlockAttributeBuffer();
		hr = dxMesh->SetAttributeTable( attrs, ngroups );
		delete[] attrs;
	}

	return dxMesh;
}

