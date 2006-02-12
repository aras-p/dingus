// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableOrderedBillboards.h"

#include "Vertices.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



CRenderableOrderedBillboards::CRenderableOrderedBillboards(
	CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName )
:	mIB( &ib ), mVBSource( sizeof(TVertex) ),
	mTexParamName( texParamName )
{
	// add self as a listener
	addListener( *this );
	mChunk = 0;
}

CRenderableOrderedBillboards::~CRenderableOrderedBillboards()
{
	removeListener( *this );
}


// ------------------------------------------------------------------
//  Rendering

void CRenderableOrderedBillboards::beforeRender( CRenderable& r, CRenderContext& ctx )
{
	assert( !mChunk );
	if( mBills.empty() )
		return;
	
	// render into VB
	mChunk = mVBSource.lock( static_cast<unsigned int>(mBills.size()) * 4 ); // 4 verts per billboard
	TVertex* vb = reinterpret_cast<TVertex*>( mChunk->getData() );
	TBillVector::const_iterator bit, bitEnd = mBills.end();
	for( bit = mBills.begin(); bit != bitEnd; ++bit ) {
		const SOBillboard& bill = *bit;
		vb->p.x		= bill.x1; vb->p.y = bill.y1; vb->p.z = 0.1f; // TBD
		vb->diffuse	= bill.color;
		vb->tu		= bill.tu1; vb->tv = bill.tv1;
		++vb;
		vb->p.x		= bill.x2; vb->p.y = bill.y1; vb->p.z = 0.1f; // TBD
		vb->diffuse	= bill.color;
		vb->tu		= bill.tu2; vb->tv = bill.tv1;
		++vb;
		vb->p.x		= bill.x2; vb->p.y = bill.y2; vb->p.z = 0.1f; // TBD
		vb->diffuse	= bill.color;
		vb->tu		= bill.tu2; vb->tv = bill.tv2;
		++vb;
		vb->p.x		= bill.x1; vb->p.y = bill.y2; vb->p.z = 0.1f; // TBD
		vb->diffuse	= bill.color;
		vb->tu		= bill.tu1; vb->tv = bill.tv2;
		++vb;
	}
	mChunk->unlock( 0 );
}

void CRenderableOrderedBillboards::afterRender( CRenderable& r, CRenderContext& ctx )
{
	mChunk = 0;
}


void CRenderableOrderedBillboards::render( const CRenderContext& ctx )
{
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();

	if( mBills.empty() )
		return;
	assert( mChunk );
	
	// set IB/VB
	assert( mIB );
	device.setIndexBuffer( mIB );
	device.setVertexBuffer( 0, &mChunk->getBuffer(), 0, static_cast<UINT>(mChunk->getStride()) );
	device.setDeclarationFVF( FVF_XYZ_DIFFUSE_TEX1 );

	// render pieces of billboards
	CD3DTexture* texture = mBills[0].texture;
	int texStart = 0;
	unsigned int n = (unsigned int)mBills.size();
	for( unsigned int i = 1; i < n; ++i ) { // from second one
		const SOBillboard& b = mBills[i];
		if( b.texture == texture )
			continue;
		// set texture on effect
		assert( texture );
		// TBD: better supply handle, not name - would be faster
		// TBD: refactor
		getParams().getEffect()->getObject()->SetTexture( mTexParamName, texture->getObject() );
		getParams().getEffect()->commitParams();

		// draw portion
		dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
			static_cast<UINT>(mChunk->getOffset()) + texStart*4, 0, (i-texStart)*4, 0, (i-texStart)*2 );

		// new texture
		texture = b.texture;
		texStart = i;

		// stats
		stats.incDrawCalls();
	}
	// last portion
	// set texture on effect
	assert( texture );
	// TBD: better supply handle, not name - would be faster
	// TBD: refactor
	getParams().getEffect()->getObject()->SetTexture( mTexParamName, texture->getObject() );
	getParams().getEffect()->commitParams();

	// draw portion
	dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
		static_cast<INT>(mChunk->getOffset()) + texStart*4, 0, (n-texStart)*4, 0, (n-texStart)*2 );
	// stats
	stats.incDrawCalls();

	// stats
	stats.incVerticesRendered( static_cast<int>(mChunk->getSize()) );
	stats.incPrimsRendered( static_cast<int>(mChunk->getSize()/2) );
}
