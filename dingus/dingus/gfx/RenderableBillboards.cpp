// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableBillboards.h"

#include "Vertices.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


const int SLOT_POOLLET_SIZE = 16;


CRenderableBillboards::CRenderableBillboards(
	CD3DIndexBuffer& ib, CEffectParams::TParamName texParamName )
:	mIB( &ib ), mVBSource( sizeof(TVertex) ),
	mTexParamName( texParamName )
{
	// add self as a listener
	addListener( *this );
}

CRenderableBillboards::~CRenderableBillboards()
{
	removeListener( *this );
}



CRenderableBillboards::TSlot& CRenderableBillboards::getSlot( CD3DTexture& tex )
{
	TTextureSlotMap::iterator it = mSlots.find( &tex );
	if( it == mSlots.end() ) {
		it = mSlots.insert( std::make_pair( &tex, TSlot(SLOT_POOLLET_SIZE) ) ).first;
	}
	return it->second;
}


SBillboard& CRenderableBillboards::addBill( CD3DTexture& texture )
{
	TSlot& slot = getSlot( texture );
	SBillboard& bill = slot.add();
	bill.texture = &texture;
	bill.discarded = false;
	return bill;
}


// ------------------------------------------------------------------
//  Rendering


void CRenderableBillboards::beforeRender( CRenderable& r, CRenderContext& ctx )
{
	assert( mChunks.empty() );
	
	// render into VB
	TTextureSlotMap::iterator it, itEnd = mSlots.end();
	for( it = mSlots.begin(); it != itEnd; ++it ) {
		CD3DTexture* tex = it->first;
		assert( tex );
		TSlot& slot = it->second;
		if( slot.empty() )
			continue;

		TVBChunk::TSharedPtr chunk = mVBSource.lock( slot.size() * 4 ); // 4 verts per billboard
		TVertex* vb = reinterpret_cast<TVertex*>( chunk->getData() );
		TSlot::iterator bit, bitEnd = slot.end();
		int ttt= 0;
		for( bit = slot.begin(); bit != bitEnd; /**/ ) {
			const SBillboard& bill = *bit;
			// if discarded - remove
			if( bill.discarded ) {
				bit = slot.erase(bit);
				bitEnd = slot.end();
				continue;
			}
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
			++bit;
			++ttt;
		}
		chunk->unlock( slot.size()*4 ); // we may have not filled discarded bills

		mChunks.push_back( std::make_pair( tex, chunk ) );
	}
}

void CRenderableBillboards::afterRender( CRenderable& r, CRenderContext& ctx )
{
	mChunks.clear();
}


void CRenderableBillboards::render( const CRenderContext& ctx )
{
	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	
	// set IB
	assert( mIB );
	device.setIndexBuffer( mIB );

	// render all chunks
	TTextureChunkVector::const_iterator it, itEnd = mChunks.end();
	for( it = mChunks.begin(); it != itEnd; ++it ) {
		CD3DTexture* tex = it->first;
		const TVBChunk& chunk = *it->second;

		// set texture on effect
		// TBD: better supply handle, not name - would be faster
		// TBD: refactor
		getParams().getEffect()->getObject()->SetTexture( mTexParamName, tex->getObject() );
		getParams().getEffect()->commitParams();

		// set streams, VB, etc.
		CD3DVertexBuffer& vb = chunk.getBuffer();
		device.setVertexBuffer( 0, &vb, 0, chunk.getStride() );
		device.setDeclarationFVF( FVF_XYZ_DIFFUSE_TEX1 );
		dx.DrawIndexedPrimitive( D3DPT_TRIANGLELIST, chunk.getOffset(), 0, chunk.getSize(), 0, chunk.getSize()/2 );
		
		// stats
		stats.incDrawCalls();
		stats.incVerticesRendered( chunk.getSize() );
		stats.incPrimsRendered( chunk.getSize()/2 );
	}
}
