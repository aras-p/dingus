// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_EMITTERS_H
#define __PARTICLE_EMITTERS_H


#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticlePhysics.h"
#include "../Vertices.h"
#include "../../kernel/Proxies.h"


namespace dingus {


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticlePhysicalEmitter : public IParticleEmitter<PARTICLE> {
public:
	typedef IParticlePhysics<PARTICLE>	TPhysics;

public:
	CParticlePhysicalEmitter( TPhysics& physics, float partsPerSec, PARTICLE const& templatePart, const ITimeSource& timer )
		: mPhysics(&physics), mPartsPerSec(partsPerSec), mTemplateParticle(templatePart), mAccumParticles(0), mTimer(&timer) { }

	virtual void emit( TParticleSystem& target ) {
		mAccumParticles += mPartsPerSec * mTimer->getDeltaTimeS();
		for( ; mAccumParticles >= 1; --mAccumParticles ) {
			PARTICLE p = mTemplateParticle;
			mPhysics->applyTo( p );
			target.addParticle( p );
		}
	}

	void setPartsPerSec( float pps ) { mPartsPerSec = pps; }
	float getPartsPerSec() const { return mPartsPerSec; }

	PARTICLE const& getTemplateParticle() const { return mTemplateParticle; }
	PARTICLE& getTemplateParticle() { return mTemplateParticle; }

private:
	PARTICLE	mTemplateParticle;
	const ITimeSource*	mTimer;
	TPhysics*	mPhysics;
	float		mPartsPerSec;
	float		mAccumParticles;
};


// --------------------------------------------------------------------------


template< typename PARTICLE >
class CParticleMeshEmitter : public IParticleEmitter<PARTICLE> {
public:
	typedef IParticlePhysics<PARTICLE>	TPhysics;
	typedef std::vector<CD3DXMesh*>		TMeshVector;

public:
	CParticleMeshEmitter( TPhysics& physics, const PARTICLE& templatePart )
		: mPhysics(&physics), mTemplateParticle(templatePart) { }

	virtual void emit( TParticleSystem& target ) {
		if( mMeshVector.empty() )
			return;

		CD3DXMesh* mesh = mMeshVector.back();
		mMeshVector.pop_back();
		assert( mesh );
		
		DWORD fvf = mesh->getObject()->GetFVF();
		if( fvf && D3DFVF_XYZ != D3DFVF_XYZ )
			return;
		
		HRESULT hRes;
		byte* data;
		hRes = mesh->getObject()->LockVertexBuffer( D3DLOCK_READONLY, &data );
		
		for( int q = 0; q < mesh->GetNumVertices(); q++ ) {
			PARTICLE p( mTemplateParticle );
			p.getPosition() += ((TVertex*)data)->p;
			
			mPhysics->applyTo( p );
			target.addParticle( p );

			data += mesh->getObject()->GetNumBytesPerVertex();
		}

		mesh->getObject()->UnlockVertexBuffer();
	}
	
	virtual void push( CD3DXMesh& mesh ) { mMeshVector.push_back( &mesh ); }

	PARTICLE const& getTemplateParticle() const { return mTemplateParticle; };
	PARTICLE& getTemplateParticle() { return mTemplateParticle; };

protected:
	typedef SVertexXyz	TVertex;

private:
	TMeshVector	mMeshVector;
	TPhysics*	mPhysics;
	PARTICLE	mTemplateParticle;
};


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CParticlePositionListEmitter : public IParticleEmitter<PARTICLE> {
public:
	typedef IParticlePhysics<PARTICLE>	TPhysics;
	typedef fastvector<SVector3>		TPositionVector;

public:
	CParticlePositionListEmitter( TPhysics& physics, PARTICLE const& templatePart )
		: mPhysics(&physics), mTemplateParticle(templatePart) { }

	virtual void emit( TParticleSystem& target ) {
		TPositionVector::const_iterator it, itEnd = mPositions.end();
		for( it = mPositions.begin(); it != mPositions.end(); ++it ) {
			PARTICLE p = mTemplateParticle;
			p.getPosition() = *it;
			mPhysics->applyTo( p );
			target.addParticle( p );
		}
		mPositions.clear();
	}

	void addPosition( const SVector3& p ) { mPositions.push_back( p ); }
	int getPositionCount() const { return mPositions.size(); }

	PARTICLE const& getTemplateParticle() const { return mTemplateParticle; }
	PARTICLE& getTemplateParticle() { return mTemplateParticle; }

private:
	TPhysics*		mPhysics;
	PARTICLE		mTemplateParticle;
	TPositionVector	mPositions;
};



}; // namespace

#endif
