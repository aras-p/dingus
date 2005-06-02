#include "stdafx.h"
#include "EntityInfoRenderer.h"
#include <dingus/gfx/Vertices.h>
#include <dingus/gfx/geometry/DynamicVBManager.h>

typedef SVertexXyzDiffuseTex1	TVertex;


CEntityInfoRenderer::CEntityInfoRenderer( CD3DIndexBuffer& ib )
:	mIB( &ib )
{
	CVertexFormat vformat(
		CVertexFormat::V_POSITION | CVertexFormat::COLOR_MASK | CVertexFormat::V_UV0_2D
	);
	mVDecl = RGET_VDECL( vformat );

	mRenderable = new CRenderableIndexedBuffer( NULL, 3 );
	mRenderable->getParams().setEffect( *RGET_FX("minimap") );
	mRenderable->getParams().addTexture( "tBase", *RGET_TEX("Hud") );
	mRenderable->setVertexDecl( mVDecl );
	mRenderable->setIB( *mIB );
	mRenderable->setStride( sizeof(TVertex), 0 );
}

CEntityInfoRenderer::~CEntityInfoRenderer()
{
	delete mRenderable;
}


void CEntityInfoRenderer::render()
{
	// early out if nothing to render
	if( mBars.empty() && mBrackets.empty() )
		return;

	const SVector3 camx = G_RENDERCTX->getCamera().getCameraMatrix().getAxisX();
	const SVector3 camy = G_RENDERCTX->getCamera().getCameraMatrix().getAxisY();

	const float BAR_HGT_RATIO = 0.25f;

	int i;

	int nbars = mBars.size();
	const int MAX_BARS = 1000; // cap the max. bars
	if( nbars > MAX_BARS )
		nbars = MAX_BARS;
	
	int nbrax = mBrackets.size();
	const int MAX_BRACKETS = 40; // cap the max. brackets
	if( nbrax > MAX_BRACKETS )
		nbrax = MAX_BRACKETS;
	
	// stuff into vertex buffer
	int vertsNeeded = nbars*8 + nbrax*4;
	TVBChunk::TSharedPtr chunk = CDynamicVBManager::getInstance().allocateChunk( vertsNeeded, sizeof(TVertex) );
	TVertex* vb = (TVertex*)chunk->getData();

	// bars
	for( i = 0; i < nbars; ++i ) {
		const SBar& e = mBars[i];
		SVector3 dxx = camx * e.size;
		SVector3 dyy = camy * (e.size*BAR_HGT_RATIO);
		SVector3 p = e.pos + camy * e.dy;
		float fillx = e.fill*2-1;
		// first quad - filled portion
		vb[0].p = p - dxx       - dyy; vb[0].diffuse = e.color; vb[0].tu = 0.0625f; vb[0].tv = 0.0625f;
		vb[1].p = p + dxx*fillx - dyy; vb[1].diffuse = e.color; vb[1].tu = 0.0625f; vb[1].tv = 0.0625f;
		vb[2].p = p + dxx*fillx + dyy; vb[2].diffuse = e.color; vb[2].tu = 0.0625f; vb[2].tv = 0.0625f;
		vb[3].p = p - dxx       + dyy; vb[3].diffuse = e.color; vb[3].tu = 0.0625f; vb[3].tv = 0.0625f;
		vb += 4;
		// second quad - border portion
		D3DCOLOR col = e.color & 0xFF000000;
		vb[0].p = p - dxx - dyy; vb[0].diffuse = col; vb[0].tu = 0.0f; vb[0].tv = 0.125f;
		vb[1].p = p + dxx - dyy; vb[1].diffuse = col; vb[1].tu = 0.5f; vb[1].tv = 0.125f;
		vb[2].p = p + dxx + dyy; vb[2].diffuse = col; vb[2].tu = 0.5f; vb[2].tv = 0.25f;
		vb[3].p = p - dxx + dyy; vb[3].diffuse = col; vb[3].tu = 0.0f; vb[3].tv = 0.25f;
		vb += 4;
	}
	// brackets
	for( i = 0; i < nbrax; ++i ) {
		const SBracket& e = mBrackets[i];
		SVector3 dxx = camx * e.size;
		SVector3 dyy = camy * e.size;
		SVector3 p = e.pos + camy * e.dy;
		const float PIX1 = 1.0f/128.0f;
		vb[0].p = p - dxx - dyy; vb[0].diffuse = e.color; vb[0].tu = 0.5f+PIX1; vb[0].tv = 0.0f+PIX1;
		vb[1].p = p + dxx - dyy; vb[1].diffuse = e.color; vb[1].tu = 1.0f-PIX1; vb[1].tv = 0.0f+PIX1;
		vb[2].p = p + dxx + dyy; vb[2].diffuse = e.color; vb[2].tu = 1.0f-PIX1; vb[2].tv = 0.5f-PIX1;
		vb[3].p = p - dxx + dyy; vb[3].diffuse = e.color; vb[3].tu = 0.5f+PIX1; vb[3].tv = 0.5f-PIX1;
		vb += 4;
	}
	chunk->unlock();

	// set onto renderable buffer and attach it
	mRenderable->setVB( chunk->getBuffer(), 0 );
	mRenderable->setBaseVertex( chunk->getOffset() );
	mRenderable->setMinVertex( 0 );
	mRenderable->setNumVertices( chunk->getSize() );
	mRenderable->setStartIndex( 0 );
	mRenderable->setPrimCount( chunk->getSize()/2 );
	mRenderable->setPrimType( D3DPT_TRIANGLELIST );

	G_RENDERCTX->attach( *mRenderable );
}
