#include "stdafx.h"
#include "StreamImpostorsRenderer.h"
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>

typedef SVertexXyzDiffuseTex1	TVertex;


CStreamImpostorsRenderer::CStreamImpostorsRenderer( CD3DIndexBuffer& ib )
:	mIB( &ib )
{
	mRenderable = new CRenderableIndexedBuffer( NULL, 2 );
	mRenderable->getParams().setEffect( *RGET_FX("StreamImpostor") );
	mRenderable->getParams().addTexture( "tBase", *RGET_TEX("StreamImpostors") );

	CVertexFormat vformat(
		CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK | CVertexFormat::V_UV0_2D
	);
	mVDecl = RGET_VDECL( vformat );
}

CStreamImpostorsRenderer::~CStreamImpostorsRenderer()
{
	delete mRenderable;
}


void CStreamImpostorsRenderer::render()
{
	// early out if no entities to render
	if( mEntities.empty() )
		return;

	// stuff entities into vertex buffer
	int n = mEntities.size();
	const int MAX_ENTITIES = 3000; // cap the max. entities
	if( n > MAX_ENTITIES )
		n = MAX_ENTITIES;
	TVBChunk::TSharedPtr chunk = CDynamicVBManager::getInstance().allocateChunk( n*4, sizeof(TVertex) );
	TVertex* vb = (TVertex*)chunk->getData();
	for( int i = 0; i < n; ++i ) {
		const SEntity& e = mEntities[i];
		const float HSIZE = 1.75f;
		SVector3 dx = e.matrix->getAxisX() * HSIZE;
		SVector3 dy = e.matrix->getAxisZ() * HSIZE;
		D3DCOLOR col = 0x00ffffff;
		col |= (int)(e.alpha*255.0f) << 24;

		vb[0].p = e.matrix->getOrigin() - dx + dy;
		vb[0].diffuse = col; vb[0].tu = 0.5f*e.type; vb[0].tv = 0.0f;
		vb[1].p = e.matrix->getOrigin() + dx + dy;
		vb[1].diffuse = col; vb[1].tu = 0.5f*e.type+0.5f; vb[1].tv = 0.0f;
		vb[2].p = e.matrix->getOrigin() + dx - dy;
		vb[2].diffuse = col; vb[2].tu = 0.5f*e.type+0.5f; vb[2].tv = 1.0f;
		vb[3].p = e.matrix->getOrigin() - dx - dy;
		vb[3].diffuse = col; vb[3].tu = 0.5f*e.type; vb[3].tv = 1.0f;
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
