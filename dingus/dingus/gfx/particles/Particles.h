// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLES_STUFF_H
#define __PARTICLES_STUFF_H

#include "../../kernel/Proxies.h"
#include "ParticleSystem.h"
#include "VBParticleRenderer.h"
#include "../geometry/VBManagerSource.h"
#include "../../renderer/RenderableBuffer.h"
#include "../../renderer/RenderContext.h"


namespace dingus {


template< typename PART >
class CParticles : public IRenderListener, public boost::noncopyable {
public:
	typedef CParticleSystem<PART>	TPSystem;
	typedef IParticlePhysics<PART>	TPPhysics;
	typedef CAbstractParticleRenderer<PART>	TPRenderer;

public:
	CParticles( TPPhysics& physics, TPRenderer& renderer,
		CD3DIndexBuffer& ib, int renderPriority, const ITimeSource& timer );
	CParticles( TPPhysics& physics, TPPhysics& postUpdatePhysics, TPRenderer& renderer,
		CD3DIndexBuffer& ib, int renderPriority, const ITimeSource& timer );
	virtual ~CParticles();

	/**
	 *  Updates particles.
	 */
	void	updateParticles() { mParticleSystem.update(); }

	/**
	 *  Renders particles, sets up geometry params on Renderable.
	 */
	void	renderParticles( const CRenderCamera& camera );

	const TPSystem& getParticleSystem() const { return mParticleSystem; }
	TPSystem& getParticleSystem() { return mParticleSystem; }

	const CRenderable& getRenderable() const { return *mRenderable; }
	CRenderable& getRenderable() { return *mRenderable; }

	// IRenderListener
	virtual void beforeRender( CRenderable& r, CRenderContext& ctx ) { renderParticles(ctx.getCamera()); }
	virtual void afterRender( CRenderable& r, CRenderContext& ctx ) { }

private:
	TPSystem			mParticleSystem;
	TPRenderer*			mParticleRenderer;
	CD3DIndexBuffer*	mIB;

	CRenderableIndexedBuffer*	mRenderable;
};


// --------------------------------------------------------------------------
//  Implementation


template< typename PART >
CParticles<PART>::CParticles( TPPhysics& physics, TPRenderer& renderer, CD3DIndexBuffer& ib, int renderPriority, const ITimeSource& timer )
:	mParticleSystem( physics, timer ),
	mParticleRenderer( &renderer ),
	mIB( &ib ),
	mRenderable( NULL )
{
	mRenderable = new CRenderableIndexedBuffer( NULL, renderPriority );
	mRenderable->addListener( *this );
}

template< typename PART >
CParticles<PART>::CParticles( TPPhysics& physics, TPPhysics& postUpdatePhysics, TPRenderer& renderer, CD3DIndexBuffer& ib, int renderPriority, const ITimeSource& timer )
:	mParticleSystem( physics, postUpdatePhysics, timer ),
	mParticleRenderer( &renderer ),
	mIB( &ib ),
	mRenderable( NULL )
{
	mRenderable = new CRenderableIndexedBuffer( NULL, renderPriority );
	mRenderable->addListener( *this );
}

template< typename PART >
CParticles<PART>::~CParticles()
{
	delete mRenderable;
}

template< typename PART >
void CParticles<PART>::renderParticles( const CRenderCamera& camera )
{
	assert( mRenderable );
	assert( mIB );

	mRenderable->resetVBs();

	if( mParticleSystem.getParticleCount() < 1 )
		return;

	// render to buffers
	CVBManagerSource vbSource( mParticleRenderer->getStride() );

	mParticleRenderer->begin(
		camera.getEye3(), camera.getCameraRotMatrix(),
		vbSource, mParticleSystem.getParticleCount()
	);
	mParticleSystem.render( *mParticleRenderer );
	CVBChunk::TSharedPtr chunk = mParticleRenderer->end();

	// set params to renderable
	mRenderable->setIB( *mIB );
	mRenderable->setVB( chunk->getVB(), 0 );
	mRenderable->setStride( chunk->getStride(), 0 );

	mRenderable->setBaseVertex( chunk->getOffset() );
	mRenderable->setMinVertex( 0 );
	mRenderable->setNumVertices( chunk->getSize() );
	mRenderable->setStartIndex( 0 );
	mRenderable->setPrimCount( chunk->getSize()/2 );
	mRenderable->setPrimType( mParticleRenderer->getPrimitiveType() );
}


}; // namespace

#endif
