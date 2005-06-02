#include "stdafx.h"
#include "MinimapRenderer.h"
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>

typedef SVertexXyzDiffuseTex1	TVertex;

CMinimapRenderer::CMinimapRenderer( CD3DIndexBuffer& ib )
:	mIB( &ib )
{
	mRenderable = new CRenderableIndexedBuffer( NULL, 2 );
	mRenderable->getParams().setEffect( *RGET_FX("minimap") );
	mRenderable->getParams().addTexture( "tBase", *RGET_TEX("Blob") );

	CVertexFormat vformat(
		CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK | CVertexFormat::V_UV0_2D
	);
	mVDecl = RGET_VDECL( vformat );
}

CMinimapRenderer::~CMinimapRenderer()
{
	delete mRenderable;
}


void CMinimapRenderer::render()
{
	// early out if no entities to render
	if( mEntities.empty() )
		return;

	// stuff entities into vertex buffer
	int n = mEntities.size();
	const int MAX_ENTITIES = 1000; // cap the max. entities
	if( n > MAX_ENTITIES )
		n = MAX_ENTITIES;
	TVBChunk::TSharedPtr chunk = CDynamicVBManager::getInstance().allocateChunk( n*4, sizeof(TVertex) );
	TVertex* vb = (TVertex*)chunk->getData();
	for( int i = 0; i < n; ++i ) {
		const SEntity& e = mEntities[i];
		vb[0].p = e.pos + SVector3(-e.size,0,-e.size);	vb[0].diffuse = e.color; vb[0].tu = 0; vb[0].tv = 0;
		vb[1].p = e.pos + SVector3( e.size,0,-e.size);	vb[1].diffuse = e.color; vb[1].tu = 1; vb[1].tv = 0;
		vb[2].p = e.pos + SVector3( e.size,0, e.size);	vb[2].diffuse = e.color; vb[2].tu = 1; vb[2].tv = 1;
		vb[3].p = e.pos + SVector3(-e.size,0, e.size);	vb[3].diffuse = e.color; vb[3].tu = 0; vb[3].tv = 1;
		vb += 4;
	}
	chunk->unlock();

	// set onto renderable buffer and attach it
	mRenderable->resetVBs();
	mRenderable->setVertexDecl( mVDecl );
	mRenderable->setIB( *mIB );
	mRenderable->setVB( chunk->getBuffer(), 0 );
	mRenderable->setStride( chunk->getStride(), 0 );

	mRenderable->setBaseVertex( chunk->getOffset() );
	mRenderable->setMinVertex( 0 );
	mRenderable->setNumVertices( chunk->getSize() );
	mRenderable->setStartIndex( 0 );
	mRenderable->setPrimCount( chunk->getSize()/2 );
	mRenderable->setPrimType( D3DPT_TRIANGLELIST );

	G_RENDERCTX->attach( *mRenderable );
}
