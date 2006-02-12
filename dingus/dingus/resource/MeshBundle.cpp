// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "MeshBundle.h"
#include "VertexDeclBundle.h"
#include "MeshSerializer.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"
#include "../utils/StringHelper.h"

using namespace dingus;


CMeshBundle::CMeshBundle()
{
	addExtension( ".dmesh" );
	addExtension( ".x" ); // legacy
};


bool CMeshBundle::loadMesh( const CResourceId& id, const CResourceId& fullName, CMesh& mesh ) const
{
	// try to load with D3DX
	// obsolete case: .X files
	if( CStringHelper::endsWith( fullName.getUniqueName(), ".x" ) || CStringHelper::endsWith( fullName.getUniqueName(), ".X" ) ) {
		ID3DXBuffer* adjancency = NULL;
		ID3DXBuffer* material = NULL;
		ID3DXBuffer* effects = NULL;
		DWORD matCount;
		ID3DXMesh* dxmesh = NULL;

		HRESULT hres = D3DXLoadMeshFromX(
			fullName.getUniqueName().c_str(),
			D3DXMESH_SYSTEMMEM,
			&CD3DDevice::getInstance().getDevice(),
			&adjancency,
			&material,
			&effects,
			&matCount,
			&dxmesh );
		if( !SUCCEEDED( hres ) )
			return false;
		assert( dxmesh );

		if( adjancency )
			adjancency->Release();
		if( material )
			material->Release();
		if( effects )
			effects->Release();

		//
		// init our mesh

		assert( !mesh.isCreated() );
		// HACK - very limited
		int formatFlags = 0;
		DWORD dxFormat = dxmesh->GetFVF();
		if( dxFormat & D3DFVF_XYZ )
			formatFlags |= CVertexFormat::V_POSITION;
		if( dxFormat & D3DFVF_NORMAL )
			formatFlags |= CVertexFormat::V_NORMAL;
		if( dxFormat & D3DFVF_TEX1 )
			formatFlags |= CVertexFormat::V_UV0_2D;
		CVertexFormat vertFormat( formatFlags );
		// HACK
		int indexStride = 2;

		CD3DVertexDecl* vertDecl = RGET_VDECL( CVertexDesc( vertFormat ) );
		mesh.createResource( dxmesh->GetNumVertices(), dxmesh->GetNumFaces()*3, vertFormat, indexStride, *vertDecl, CMesh::BUF_STATIC );

		//
		// now, copy data into our mesh

		void *dxvb, *dxib;
		dxmesh->LockVertexBuffer( 0, &dxvb );
		dxmesh->LockIndexBuffer( 0, &dxib );
		void* myvb = mesh.lockVBWrite();
		void* myib = mesh.lockIBWrite();

		memcpy( myvb, dxvb, mesh.getVertexCount() * mesh.getVertexStride() );
		memcpy( myib, dxib, mesh.getIndexCount() * mesh.getIndexStride() );
		
		dxmesh->UnlockVertexBuffer();
		dxmesh->UnlockIndexBuffer();
		mesh.unlockVBWrite();
		mesh.unlockIBWrite();

		//
		// create groups

		int ngroups;
		dxmesh->GetAttributeTable( 0, (DWORD*)&ngroups );
		D3DXATTRIBUTERANGE *attrs = new D3DXATTRIBUTERANGE[ngroups];
		dxmesh->GetAttributeTable( attrs, (DWORD*)&ngroups );
		for( int i = 0; i < ngroups; ++i ) {
			const D3DXATTRIBUTERANGE& a = attrs[i];
			mesh.addGroup( CMesh::CGroup( a.VertexStart, a.VertexCount, a.FaceStart, a.FaceCount ) );
		}
		delete[] attrs;

		// release d3dx mesh
		dxmesh->Release();

	} else {

		// our own format
		//assert( !mesh.isCreated() );
		bool ok = CMeshSerializer::loadMeshFromFile( fullName.getUniqueName().c_str(), mesh );
		if( !ok )
			return false;
	}
	mesh.computeAABBs();
	CONSOLE.write( "mesh loaded '" + id.getUniqueName() + "'" );
	return true;
}


CMesh* CMeshBundle::loadResourceById( const CResourceId& id, const CResourceId& fullName )
{
	CMesh* mesh = new CMesh();
	bool ok = loadMesh( id, fullName, *mesh );
	if( !ok ) {
		delete mesh;
		return NULL;
	}
	return mesh;
}

void CMeshBundle::createResource()
{
	const TStringVector& ext = getExtensions();
	const TStringVector& dir = getDirectories();
	size_t ne = ext.size();
	size_t nd = dir.size();

	// reload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CMesh& res = *it->second;
		assert( !res.isCreated() );
		// try all directories
		for( size_t d = 0; d < nd; ++d ) {
			// try all extensions
			for( size_t e = 0; e < ne; ++e ) {
				CResourceId fullid( dir[d] + it->first.getUniqueName() + ext[e] );
				bool ok = loadMesh( it->first, fullid, res );
				if( ok )
					break;
			}
		}
		// maybe ID was full name already
		loadMesh( it->first, it->first, res );
		assert( res.isCreated() );
	}
}

void CMeshBundle::activateResource()
{
}

void CMeshBundle::passivateResource()
{
}

void CMeshBundle::deleteResource()
{
	// unload all objects
	TResourceMap::iterator it;
	for( it = mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
		CMesh& res = *it->second;
		assert( res.isCreated() );
		res.deleteResource();
	}
}
