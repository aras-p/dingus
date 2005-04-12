// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableMesh.h"
#include "RenderContext.h"
#include "../utils/Errors.h"
#include "../kernel/D3DDevice.h"


using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CRenderableMesh,256,false);


CRenderableMesh::CRenderableMesh( CMesh& mesh, int group, const SVector3* origin, int priority )
:	CRenderable( origin, priority ),
	mMesh( &mesh ),
	mGroup( group )
{
	assert( group >= 0 && group < mesh.getGroupCount() || group == ALL_GROUPS );
	assert( mesh.isCreated() );
}


void CRenderableMesh::render( CRenderContext const& ctx )
{
	assert( mMesh );

	HRESULT hr;

	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();

	device.setIndexBuffer( &mMesh->getIB() );
	device.setVertexBuffer( 0, &mMesh->getVB(), 0, mMesh->getVertexStride() );
	device.setDeclaration( mMesh->getVertexDecl() );

	if( mGroup == ALL_GROUPS ) {
		hr = dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
			0, 0, mMesh->getVertexCount(),
			0, mMesh->getIndexCount()/3 );
		stats.incVerticesRendered( mMesh->getVertexCount() );
		stats.incPrimsRendered( mMesh->getIndexCount()/3 );
	} else {
		const CMesh::CGroup& group = mMesh->getGroup( mGroup );
		hr = dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
			0, group.getFirstVertex(), group.getVertexCount(),
			group.getFirstPrim() * 3, group.getPrimCount() );
		stats.incVerticesRendered( group.getVertexCount() );
		stats.incPrimsRendered( group.getPrimCount() );
	}
	if( FAILED( hr ) ) {
		THROW_DXERROR( hr, "failed to DIP" );
	}

	// stats
	stats.incDrawCalls();
}
