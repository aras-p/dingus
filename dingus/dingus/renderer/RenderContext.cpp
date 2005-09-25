// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderContext.h"
#include "Renderable.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;



CRenderContext::CRenderContext( CD3DXEffect& globalFx )
:	mGlobalEffect( &globalFx ),
	mInsideDirect( false ),
	mDirectCurrFX( NULL )
{
	assert( mGlobalEffect );
	mGlobalParams.setEffect( *mGlobalEffect );

	// reserve some space in renderables
	mRenderables.reserve( 256 );

	// add-by-ref view & projection matrices to the global params
	mGlobalParams.addMatrix4x4Ref( "mView", mRenderCamera.getViewMatrix() );
	mGlobalParams.addMatrix4x4Ref( "mProjection", mRenderCamera.getProjectionMatrix() );
	mGlobalParams.addMatrix4x4Ref( "mViewProj", mRenderCamera.getViewProjMatrix() );
	mGlobalParams.addVector3Ref( "vEye", mRenderCamera.getCameraMatrix().getOrigin() );
	mGlobalParams.addVector3Ref( "vCameraX", mRenderCamera.getCameraMatrix().getAxisX() );
	mGlobalParams.addVector3Ref( "vCameraY", mRenderCamera.getCameraMatrix().getAxisY() );
}



// --------------------------------------------------------------------------

//
// Sorter for renderables. Takes care of sort-from-back, inter-effects sort, etc.

struct SRenderableSorter {
	const CRenderContext* ctx;

	bool operator()( const CRenderable* ra, const CRenderable* rb ) {
		assert( ra && rb );

		// Just shortcut
		if( ra == rb )
			return false;

		// priority
		int apri = ra->getPriority();
		int bpri = rb->getPriority();
		if( apri != bpri )
			return apri < bpri;

		const CD3DXEffect* ea = ra->getParams().getEffect();
		const CD3DXEffect* eb = rb->getParams().getEffect();

		// if effects inter-sort order
		// this puts sorted effects last, and orders effects by required
		// vertex shader version
		if( ea->getSortValue() != eb->getSortValue() ) {
			return ea->getSortValue() < eb->getSortValue();
		}

		// if back-to-front: by distance to camera
		bool aback = ea->isBackToFrontSorted();
		bool bback = eb->isBackToFrontSorted();
		if( aback && bback ) {
			const SVector3* aor = ra->getOrigin();
			const SVector3* bor = rb->getOrigin();
			if( aor && bor ) {
				const SVector3& eye = ctx->getCamera().getEye3();
				float adist = SVector3(*aor-eye).lengthSq();
				float bdist = SVector3(*bor-eye).lengthSq();
				
				// NOTE
				// Should be this, but VC seems to generate somewhat buggy
				// code - sometimes returns true even if equal. This doesn't
				// happen in Debug build... Spent 3 days on this! :(

				//return adist > bdist;
				return adist - bdist > 0.00001f;
			}
			// Strange case - should be sorted, but one/both don't supply
			// origins. So just order them arbitrarily (by pointers).
			return ra < rb;
		}
		// These cases should be caught in effect's "sort value"
		assert( !aback && !bback );

		// by effect - just compare pointers :)
		if( ea != eb )
			return ea < eb;

		// by used IB
		const CD3DIndexBuffer* iba = ra->getUsedIB();
		const CD3DIndexBuffer* ibb = rb->getUsedIB();
		if( iba != ibb )
			return iba < ibb;

		// by used VB
		const CD3DVertexBuffer* vba = ra->getUsedVB();
		const CD3DVertexBuffer* vbb = rb->getUsedVB();
		if( vba != vbb )
			return vba < vbb;

		// just by pointers...
		return ra < rb;
	};
};

// --------------------------------------------------------------------------

void CRenderContext::applyGlobalEffect()
{
	// global params and effect
	mGlobalParams.applyToEffect();
	int passes = mGlobalEffect->beginFx();
	mGlobalEffect->beginPass( 0 );
	mGlobalEffect->endPass();
	mGlobalEffect->endFx();
}

// --------------------------------------------------------------------------

void CRenderContext::sortRenderables()
{
	SRenderableSorter sorter;
	sorter.ctx = this;
	std::sort( mRenderables.begin(), mRenderables.end(), sorter );
}

void CRenderContext::renderRenderables()
{
	CD3DDevice& device = CD3DDevice::getInstance();

	//
	// turn SWVP off (just in case)

	bool mixedVP = (device.getCaps().getVertexProcessing() == CD3DDeviceCaps::VP_MIXED);
	if( mixedVP )
		device.getDevice().SetSoftwareVertexProcessing( false );

	// here we have our renderables attached and sorted by (with the exception
	// of back-to-front):
	//	1. priority,
	//	2. need to sort back-to-front (unsorted, then sorted)
	//	3. vertex shader req. (in ascending order - none, 1.1, 2.0 etc.)
	//	4. effect (in no particular order)

	CD3DXEffect*	currFX = NULL;
	bool			currSWVP = false;

	//
	// go thru all renderables

	TRenderableVector::iterator itR = mRenderables.begin();
	TRenderableVector::iterator itEnd = mRenderables.end();
	TRenderableVector::iterator itFxStart = itR;
	for( ; itR != itEnd; ++itR ) {
		CRenderable* rend = *itR;
		assert( rend );

		CEffectParams& params = rend->getParams();
		CD3DXEffect* newFX = params.getEffect();
		assert( newFX );

		//
		// check effect change. if changed, render all renderables of the old effect

		if( newFX != currFX ) {
			// render all renderables with old fx
			if( currFX ) {
				assert( currFX );

				// begin
				int passes = currFX->beginFx();
				// all passes
				for( int p = 0; p < passes; ++p ) {
					currFX->beginPass( p );
					// all objects
					TRenderableVector::iterator it;
					for( it = itFxStart; it != itR; ++it ) {
						CRenderable* r = *it;
						assert( r );
						r->beforeRender( *this );
						r->getParams().applyToEffect();
						currFX->commitParams();
						r->render( *this );
						r->afterRender( *this );
					}
					currFX->endPass();
				}
				// end old fx
				currFX->endFx();

				// stats
				device.getStats().incEffectChanges();
			}
			// set new fx
			currFX = newFX;
			itFxStart = itR;
			// check SWVP change
			bool newSWVP = newFX->isSoftwareVertexProcessed();
			if( newSWVP != currSWVP && mixedVP ) {
				currSWVP = newSWVP;
				device.getDevice().SetSoftwareVertexProcessing( newSWVP );
			}
		}
	}

	//
	// here we have to render all renderables of the last fx

	if( currFX ) {
		assert( currFX );
		// begin
		int passes = currFX->beginFx();
		// all passes
		for( int p = 0; p < passes; ++p ) {
			currFX->beginPass( p );
			// all objects
			TRenderableVector::iterator it;
			for( it = itFxStart; it != itEnd; ++it ) {
				CRenderable* r = *it;
				assert( r );
				r->beforeRender( *this );
				r->getParams().applyToEffect();
				currFX->commitParams();
				r->render( *this );
				r->afterRender( *this );
			}
			currFX->endPass();
		}
		// end old fx
		currFX->endFx();
		
		// stats
		device.getStats().incEffectChanges();
	}

	//
	// and turn SWVP off

	if( mixedVP )
		device.getDevice().SetSoftwareVertexProcessing( false );
}

void CRenderContext::perform()
{
	// sort
	sortRenderables();
	// render
	renderRenderables();
	// clear
	mRenderables.clear();
}


// --------------------------------------------------------------------------

void CRenderContext::directBegin()
{
	// turn SWVP off (just in case)
	CD3DDevice& device = CD3DDevice::getInstance();
	bool mixedVP = (device.getCaps().getVertexProcessing() == CD3DDeviceCaps::VP_MIXED);
	if( mixedVP )
		device.getDevice().SetSoftwareVertexProcessing( false );

	assert( !mInsideDirect );
	assert( !mDirectCurrFX );
	mInsideDirect = true;
	mDirectCurrFX = NULL;
	mDirectCurrPasses = 0;
}

void CRenderContext::directEnd()
{
	assert( mInsideDirect );
	if( mDirectCurrFX ) {
		// end
		mDirectCurrFX->endFx();
	}
	mInsideDirect = false;
	mDirectCurrFX = NULL;
	mDirectCurrPasses = 0;

	// turn SWVP off (just in case)
	CD3DDevice& device = CD3DDevice::getInstance();
	bool mixedVP = (device.getCaps().getVertexProcessing() == CD3DDeviceCaps::VP_MIXED);
	if( mixedVP )
		device.getDevice().SetSoftwareVertexProcessing( false );
}

void CRenderContext::directRender( CRenderable& r )
{
	CD3DXEffect* fx = r.getParams().getEffect();
	assert( fx );
	
	// effect changed?
	directSetFX( *fx );

	// apply renderable params to fx
	assert( mDirectCurrFX );
	assert( mDirectCurrFX == fx );
	r.getParams().applyToEffect();

	// renderRenderables all passes
	assert( mDirectCurrPasses > 0 );
	r.beforeRender( *this );
	for( int p = 0; p < mDirectCurrPasses; ++p ) {
		mDirectCurrFX->beginPass( p );
		r.render( *this );
		mDirectCurrFX->endPass();
	}
	r.afterRender( *this );
}

int CRenderContext::directSetFX( CD3DXEffect& fx )
{
	// effect changed?
	if( &fx != mDirectCurrFX ) {
		CD3DDevice& device = CD3DDevice::getInstance();
		// if old was - end it
		if( mDirectCurrFX ) {
			mDirectCurrFX->endFx();
			device.getStats().incEffectChanges();
		}
		// see HW/SW VP
		bool mixedVP = (device.getCaps().getVertexProcessing() == CD3DDeviceCaps::VP_MIXED);
		if( mixedVP )
			device.getDevice().SetSoftwareVertexProcessing( fx.isSoftwareVertexProcessed() );

		// begin new one
		mDirectCurrFX = &fx;
		mDirectCurrPasses = fx.beginFx();
	}
	return mDirectCurrPasses;
}

void CRenderContext::directFXPassBegin( int pass )
{
	mDirectCurrFX->beginPass( pass );
}

void CRenderContext::directFXPassEnd()
{
	mDirectCurrFX->endPass();
}
