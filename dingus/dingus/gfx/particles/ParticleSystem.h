// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_SYSTEM_H
#define __PARTICLE_SYSTEM_H

#include "ParticleEmitter.h"
#include "ParticlePhysics.h"
#include "ParticleRenderer.h"


namespace dingus {

template<typename PARTICLE>
class IParticleEmitter;


template< typename PARTICLE >
class CParticleSystem {
public:
	typedef IParticleEmitter<PARTICLE>	TEmitter;
	typedef IParticlePhysics<PARTICLE>	TPhysics;
	typedef IParticleRenderer<PARTICLE>	TRenderer;
	typedef fastvector<PARTICLE>		TParticleVector;
	typedef fastvector<TEmitter*>		TEmitterVector;
	
public:
	CParticleSystem( TPhysics& physics, const ITimeSource& timer )
		: mPhysics(&physics), mPostUpdatePhysics(0), mTimer(&timer) { }
	CParticleSystem( TPhysics& physics, TPhysics& postUpdatePhysics, const ITimeSource& timer )
		: mPhysics(&physics), mPostUpdatePhysics(&postUpdatePhysics), mTimer(&timer) { }

	PARTICLE& addParticle( const PARTICLE& particle );

	void update();
	void render( TRenderer& renderer ) { if( mParticles.empty() ) return; renderer.render(mParticles); }

	void addEmitter( TEmitter& emitter ) { mEmitters.push_back(&emitter); }
	void removeEmitter( TEmitter& emitter ) { mEmitters.remove(&emitter); }

	TParticleVector const& getParticles() const { return mParticles; }
	int getParticleCount() const { return mParticles.size(); }

private:
	TParticleVector		mParticles;
	TEmitterVector		mEmitters;
	const ITimeSource*	mTimer;
	TPhysics::TSharedPtr mPhysics;
	TPhysics::TSharedPtr mPostUpdatePhysics;	
};



template< typename PARTICLE >
PARTICLE& CParticleSystem<PARTICLE>::addParticle( const PARTICLE& particle )
{
	mParticles.push_back( particle );
	return mParticles.back();
}


template< typename PARTICLE >
void CParticleSystem<PARTICLE>::update()
{
	float dt = mTimer->getDeltaTimeS();

	//
	// emit particles

	TEmitterVector::iterator eit, eitEnd = mEmitters.end();
	for( eit = mEmitters.begin(); eit != eitEnd; ++eit ) {
		(*eit)->emit( *this );
	}
	
	// TBD: maybe update along with rendering?

	//
	// update

	TParticleVector::iterator pit, pitEnd = mParticles.end();
	for( pit = mParticles.begin(); pit != pitEnd; /* nothing */ ) {
		PARTICLE& particle = *pit;
		
		assert( mPhysics );
		mPhysics->applyTo( particle );

		particle.update( dt );

		if( mPostUpdatePhysics ) {
			mPostUpdatePhysics->applyTo( particle );
		} 
		
		// if particle is dead, remove it 
		if( !particle.isAlive() ) {
			pit = mParticles.erase( pit );
			--pitEnd;
		} else {
			++pit;
		}
	}
}


}; // namespace


#endif
