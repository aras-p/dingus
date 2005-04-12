// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VSKeyframeMeshRenderer.h"

#include "../renderer/RenderableBuffer.h"
#include "../utils/Errors.h"


using namespace dingus;


CVSKeyframe2MeshRenderer::CVSKeyframe2MeshRenderer(	CD3DVertexDecl& vDecl )
:	mVertexDecl(&vDecl)
{
}


void CVSKeyframe2MeshRenderer::setKeyframe( CRenderableIndexedBuffer& renderable, CMesh& mesh0, CMesh& mesh1, float t )
{
	renderable.resetVBs();

	// set mesh 0
	renderable.setVB( mesh0.getVB(), 0 );
	renderable.setStride( mesh0.getVertexStride(), 0 );

	// set mesh 1
	renderable.setVB( mesh1.getVB(), 1 );
	renderable.setStride( mesh1.getVertexStride(), 1 );

	// other params from mesh 0
	renderable.setIB( mesh0.getIB() );

	renderable.setPrimType( D3DPT_TRIANGLELIST );
	renderable.setBaseVertex( 0 );
	renderable.setMinVertex( 0 );
	renderable.setNumVertices( mesh0.getVertexCount() );
	renderable.setStartIndex( 0 );
	renderable.setPrimCount( mesh0.getIndexCount()/3 );

	renderable.setVertexDecl( mVertexDecl );

	mBlendParam = t;
}
