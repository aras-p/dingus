// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_PHYSICS_H
#define __PARTICLE_PHYSICS_H

#include "../../math/Vector2.h"


namespace dingus {


// --------------------------------------------------------------------------

template< typename PARTICLE >
class IParticlePhysics : public CRefCounted {
public:
	typedef IParticlePhysics<PARTICLE> this_type;
	typedef DingusSmartPtr<this_type> TSharedPtr;
public:
	virtual ~IParticlePhysics() = 0 { };
	virtual void applyTo( PARTICLE& p ) const = 0;
};


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticleCompositePhysics : public IParticlePhysics<PARTICLE> {
public:
	typedef std::vector<TSharedPtr> TPhysicsVector;
public:
	CParticleCompositePhysics() { };
	CParticleCompositePhysics( const TPhysicsVector& physics ) : mPhysics( physics ) { };

	virtual void applyTo( PARTICLE& p ) const {
		TPhysicsVector::const_iterator it, itEnd = mPhysics.end();
		for( it = mPhysics.begin(); it != itEnd; ++it ) {
			(*it)->applyTo( p );
		}
	}

	void addPhysics( this_type& p ) { mPhysics.push_back( &p ); }

private:
	TPhysicsVector	mPhysics;
};


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticleAnimPhysics : public IParticlePhysics<PARTICLE> {
public:
	typedef	PARTICLE::SState	TKey;
	typedef std::vector<TKey>	TKeyVector;

public:
	CParticleAnimPhysics()
		: mKeyCountMinusOne(-1) { };
	CParticleAnimPhysics( TKeyVector const& keys )
		: mKeys(keys), mKeyCountMinusOne(keys.size()-1) { };

	void	addKey( TKey const& k ) { mKeys.push_back(k); mKeyCountMinusOne = (float)mKeys.size()-1; }

	virtual void applyTo( PARTICLE& p ) const {
		float relAge = 0.95f - p.getTTL() * p.getInvLifetime(); // HACK: not 1.0f because of precision issues
		float index = relAge * mKeyCountMinusOne;
		if( index < 0 )
			index = 0;
		float from = floorf(index);
		float frac = index - from;
		int i1 = (int)from;

		ASSERT_MSG( i1>=0 && i1+1<mKeys.size(), "bad key index" );
		
		TKey const& k1 = mKeys[i1];
		TKey const& k2 = mKeys[i1+1];

		p.setFromState( k1, k2, frac );
	};

private:
	TKeyVector	mKeys;
	float		mKeyCountMinusOne;
};


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticleAnimatePhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleAnimatePhysics( int rows, int cols ) 
	:	mFramesMinusOne( rows * cols - 1 ) 
	{
		for( int i = 0; i <= mFramesMinusOne; i++ )
			mFrames.push_back( SVector2( 
				1.0f / cols * ( i % cols ),
				1.0f / rows * ( i / rows )
			));
	}

	virtual void applyTo( PARTICLE& p ) const {
		float relAge = 1.0f - p.getTTL() * p.getInvLifetime();
		float index = relAge * mFramesMinusOne;
		assert( index >= 0 );

		p.setTextureUV( mFrames[index].x, mFrames[index].y );
	};

private:
	typedef std::vector<SVector2> TVector2Vector;
	int mFramesMinusOne;
	TVector2Vector mFrames;
};


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticleAnimateBlendPhysics : public IParticlePhysics<PARTICLE> {
public:
	CParticleAnimateBlendPhysics( int rows, int cols ) 
	{
		mFrames.push_back( SVector2( 0, 0 ) );
		mColorFactors.push_back( 0 );
		for( int i = 0; i < rows * cols; i++ ) {
			mFrames.push_back( SVector2( 
				1.0f / cols * ( i % cols ),
				1.0f / rows * ( i / rows )
			));
			mColorFactors.push_back( 1 );
		}
		mFrames.push_back( SVector2( 0, 0 ) );
		mColorFactors.push_back( 0 );
		mFrames.push_back( SVector2( 0, 0 ) );
		mColorFactors.push_back( 0 );
		mFramesMinusTwo = mFrames.size() - 2;
	}

	virtual void applyTo( PARTICLE& p ) const {
		float relAge = 1.0f - p.getTTL() * p.getInvLifetime();
		float indexf = relAge * mFramesMinusTwo;
		int index = indexf;
		assert( index >= 0 );

		float f = index + 1 - indexf;

		p.setTextureUV1( mFrames[index].x, mFrames[index].y );
		p.setTextureUV2( mFrames[index + 1].x, mFrames[index + 1].y );

		p.setColor1Factor( mColorFactors[index] * f );
		p.setColor2Factor( mColorFactors[index + 1] * ( 1 - f ) );
	};

private:
	typedef std::vector<SVector2> TVector2Vector;
	typedef std::vector<float> TFloatVector;
	int mFramesMinusTwo;
	TVector2Vector mFrames;
	TFloatVector mColorFactors;
};


}; // namespace

#endif
