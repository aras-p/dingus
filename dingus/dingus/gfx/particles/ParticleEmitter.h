// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_EMITTER_H
#define __PARTICLE_EMITTER_H

#include "ParticleSystem.h"

namespace dingus {

template<typename PARTICLE>
class CParticleSystem;


// --------------------------------------------------------------------------

template< typename PARTICLE >
class IParticleEmitter {
public:
	typedef CParticleSystem<PARTICLE>	TParticleSystem;

public:
	virtual ~IParticleEmitter() { };

	virtual void emit( TParticleSystem& target ) = 0;
};


}; // namespace

#endif
