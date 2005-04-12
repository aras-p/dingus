// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_PHYSICSES_H
#define __PARTICLE_PHYSICSES_H

#include "ParticlePhysics.h"
#include "../../math/Vector3.h"
#include "../../utils/Random.h"


namespace dingus {


// --------------------------------------------------------------------------

/**
 *  "Do nothing" physics.
 */
template< typename PARTICLE >
class CParticleEmptyPhysics : public IParticlePhysics<PARTICLE> {
public:
	virtual void applyTo( PARTICLE& particle ) const { }
};

// --------------------------------------------------------------------------

/**
 *  Displaces particle position randomly up to given displacement limit.
 */
template< typename PARTICLE >
class CParticleDisplacePhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleDisplacePhysics( const SVector3& displace )
		: mDisplace(displace) { }

	virtual void applyTo( PARTICLE& particle ) const {
		SVector3& p = particle.getPosition();
		p.x += frandf( -mDisplace.x, mDisplace.x );
		p.y += frandf( -mDisplace.y, mDisplace.y );
		p.z += frandf( -mDisplace.z, mDisplace.z );
	}

	SVector3 const& getDisplace() const { return mDisplace; };
	SVector3& getDisplace() { return mDisplace; };

private:
	SVector3	mDisplace;
};


// --------------------------------------------------------------------------

/**
 *  Displaces particle position faked-gaussian randomly up to given displacement limit.
 */
template< typename PARTICLE >
class CParticleGaussianDisplacePhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleGaussianDisplacePhysics( float displace )
		: mDisplace( epsilon ) { }

	virtual void applyTo( PARTICLE& particle ) const {
		SVector3& p = particle.getPosition();
		p.x += frandf(1.0f) * frandf( -mDisplace, mDisplace );
		p.y += frandf(1.0f) * frandf( -mDisplace, mDisplace );
		p.z += frandf(1.0f) * frandf( -mDisplace, mDisplace );
	}

	float getDisplace() const { return mDisplace; };
	void setDisplace( float d ) { mDisplace = d; };

private:
	float		mDisplace;
};


// --------------------------------------------------------------------------

/**
 *  Repulses particles away from origin, based on squared distance.
 */
template< typename PARTICLE >
class CParticleRepulsionPhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleRepulsionPhysics( const SVector3& origin, float strength )
		: mOrigin(origin), mStrength(strength) { }

	virtual void applyTo( PARTICLE& particle ) const {
		SVector3 v = particle.getPosition() - mOrigin;
		float lenSq = v.lengthSq();
		if( lenSq < 1.0e-5f )
			lenSq = 1.0e-5f;
		float invLenSq = 1.0f / lenSq;
		particle.getVelocity() += v * (mStrength * invLenSq);
	}
	
	const SVector3& getOrigin() const { return mOrigin; };
	SVector3& getOrigin() { return mOrigin; };
	
	float getStrength() const { return mStrength; };
	void setStrength( float strength ) { mStrength = strength; };
	
private:
	SVector3	mOrigin;
	float		mStrength;
};


// --------------------------------------------------------------------------

/**
 *  Applies constant force to particle. Fake, as mass isn't taken into account.
 */
template< typename PARTICLE >
class CParticleGravityPhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleGravityPhysics( const SVector3& force, const ITimeSource& timer )
		: mForce(force), mTimer(&timer) { }

	virtual void applyTo( PARTICLE& particle ) const {
		particle.getVelocity() += mTimer->getDeltaTimeS() * mForce;
	}

	const SVector3& getForce() const { return mForce; };
	SVector3& getForce() { return mForce; };
	
private:
	SVector3			mForce;
	const ITimeSource*	mTimer;
};


// --------------------------------------------------------------------------

/*
template< typename PARTICLE >
class CDampingPhysics : public IParticlePhysics<PARTICLE>
{
public:
	CDampingPhysics( float dampingForce, const CFrameTime& frameTime )
		: mDampingForce( dampingForce ), mFrameTime( frameTime ) { }
	virtual ~CDampingPhysics() { }

	virtual void applyTo( PARTICLE& particle ) {
		SVector3 n = particle.getVelocity();
		D3DXVec3Normalize( &n, &n );
		particle.getVelocity() -= mFrameTime.getDelta() * n * mDampingForce;
	}
	
private:
	float				mDampingForce;
	const CFrameTime&	mFrameTime;
};
*/


}; // namespace

#endif
