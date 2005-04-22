#include "MeshProcessor.h"

using namespace mproc;


const D3DVERTEXELEMENT9 mproc::MESH_DECLARATION[] = {
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 }, 
	{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
	{ 0, 48, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
	{ 0, 60, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
	{ 0, 64, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 72, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	{ 0, 80, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
	{ 0, 88, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
	D3DDECL_END()
};

IDirect3DDevice9*	mproc::d3dDevice = 0;


// --------------------------------------------------------------------------



CMesh::CMesh()
:	mMesh(0), mAdjacency(0)
{
}

CMesh::CMesh( const CMesh& r )
:	mMesh(0), mAdjacency(0)
{
	assignMesh( *r.mMesh );
	if( mMesh ) {
		mMesh->AddRef();
		memcpy( mAdjacency, r.mAdjacency, mMesh->GetNumFaces()*3*4 );
	}
}

void CMesh::operator=( const CMesh& r )
{
	assignMesh( *r.mMesh );
	if( mMesh ) {
		mMesh->AddRef();
		memcpy( mAdjacency, r.mAdjacency, mMesh->GetNumFaces()*3*4 );
	}
}

CMesh::~CMesh()
{
	clearMesh();
}

void CMesh::clearMesh()
{
	if( mMesh ){
		mMesh->Release();
		mMesh = 0;
	}
	if( mAdjacency ) {
		delete[] mAdjacency;
		mAdjacency = 0;
	}
}

void CMesh::assignMesh( ID3DXMesh& mesh )
{
	clearMesh();
	mMesh = &mesh;
	if( mMesh )
		mAdjacency = new DWORD[mMesh->GetNumFaces()*3];
}

void CMesh::assignMesh( ID3DXMesh& mesh, DWORD* adjacency )
{
	clearMesh();
	mMesh = &mesh;
	mAdjacency = adjacency;
}

bool CMesh::calcAdjacency()
{
	assert( mMesh && mAdjacency );
	HRESULT hr = mMesh->GenerateAdjacency( 0.0f, mAdjacency );
	return (SUCCEEDED(hr)) ? true : false;
}

bool CMesh::weldVertices()
{
	assert( isCreated() );

	DWORD* weldedAdj = new DWORD[mMesh->GetNumFaces()*3];
	DWORD* faceRemap = new DWORD[mMesh->GetNumFaces()];
	ID3DXBuffer* weldedVertRemap = 0;
	
	D3DXWELDEPSILONS weldEps;
	memset( &weldEps, 0, sizeof(weldEps) );
	weldEps.Position = 0.0f;
	weldEps.Normal = weldEps.Tangent = weldEps.Binormal = 0.0f;
	for( int i = 0; i < UV_COUNT; ++i )
		weldEps.Texcoord[i] = 0.0f;
	weldEps.BlendWeights = 0.0f;
	HRESULT hr = D3DXWeldVertices( mMesh, 0, &weldEps, mAdjacency, weldedAdj, faceRemap, &weldedVertRemap );
	if( weldedVertRemap )
		weldedVertRemap->Release();
	if( FAILED(hr) ) {
		delete[] weldedAdj;
		delete[] faceRemap;
		return false;
	}
	delete[] mAdjacency;
	mAdjacency = weldedAdj;
	return true;
}


// --------------------------------------------------------------------------


HRESULT mproc::createMesh( uint ntris, uint nverts, CMesh& dest )
{
	dest.clearMesh();
	assert( d3dDevice );
	ID3DXMesh* m = 0;
	HRESULT hr = D3DXCreateMesh( ntris, nverts, D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, MESH_DECLARATION, d3dDevice, &m );
	if( FAILED(hr) )
		return hr;
	dest.assignMesh( *m );
	return S_OK;
}

HRESULT mproc::copyAttrBuf( CMesh& src, CMesh& dst )
{
	assert( src.isValid() && dst.isValid() );
	HRESULT hr;
	DWORD *asrc, *adst;
	hr = src.getMesh().LockAttributeBuffer( D3DLOCK_READONLY, &asrc );
	if( FAILED(hr) ) {
		return hr;
	}
	hr = dst.getMesh().LockAttributeBuffer( 0, &adst );
	if( FAILED(hr) ) {
		src.getMesh().UnlockAttributeBuffer();
		return hr;
	}
	memcpy( adst, asrc, src.getMesh().GetNumFaces() * sizeof(DWORD) );
	src.getMesh().UnlockAttributeBuffer();
	dst.getMesh().UnlockAttributeBuffer();
	return S_OK;
}

HRESULT mproc::cleanMesh( CMesh& src, CMesh& dst )
{
	assert( src.isValid() );
	dst.clearMesh();

	DWORD* adj = new DWORD[src.getMesh().GetNumFaces()*3];
	assert( adj );

	ID3DXMesh* resmesh = 0;
	ID3DXBuffer* cleanErrors = 0;
	HRESULT hr = D3DXCleanMesh( &src.getMesh(), src.getAdjacency(), &resmesh, adj, &cleanErrors );
	//hr = D3DXCleanMesh( D3DXCLEAN_OPTIMIZATION, dxmesh, adj, &resmesh, adj, &cleanErrors );
	if( cleanErrors )
		cleanErrors->Release();
	if( FAILED(hr) ) {
		delete[] adj;
		return hr;
	}
	dst.assignMesh( *resmesh, adj );
	return S_OK;
}

HRESULT mproc::optimizeMesh( CMesh& src, CMesh& dst )
{
	assert( src.isValid() );
	dst.clearMesh();

	DWORD* adj = new DWORD[src.getMesh().GetNumFaces()*3];
	assert( adj );

	ID3DXMesh* resmesh = 0;
	HRESULT hr = src.getMesh().Optimize( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE | D3DXMESHOPT_DEVICEINDEPENDENT, src.getAdjacency(), adj, NULL, NULL, &resmesh );
	if( FAILED(hr) ) {
		delete[] adj;
		return hr;
	}
	dst.assignMesh( *resmesh, adj );
	return S_OK;
}
