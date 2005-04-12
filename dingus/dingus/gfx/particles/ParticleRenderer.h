// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_RENDERER_H
#define __PARTICLE_RENDERER_H

namespace dingus {


template< typename PARTICLE >
class IParticleRenderer {
public:
	typedef fastvector<PARTICLE>	TParticleVector;

public:
	virtual ~IParticleRenderer() = 0 { };

	virtual void render( TParticleVector const& particles ) = 0;
};


}; // namespace


#endif
