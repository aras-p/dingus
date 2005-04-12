// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_PARTICLE_RENDERER_H
#define __VB_PARTICLE_RENDERER_H


#include "ParticleRenderer.h"
#include "Particle.h"
#include "ParticleRenderHelper.h"
#include "../../math/Vector3.h"
#include "../../math/Matrix4x4.h"
#include "../geometry/ChunkSource.h"
#include "../geometry/VBChunk.h"
#include "../Vertices.h"


namespace dingus {


// --------------------------------------------------------------------------

template< typename PARTICLE >
class CAbstractParticleRenderer : public IParticleRenderer<PARTICLE> {
public:
	CAbstractParticleRenderer( int stride, D3DPRIMITIVETYPE primType )
		: mStride(stride), mPrimitiveType(primType) { }
	
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount ) = 0;
	virtual CVBChunk::TSharedPtr end() = 0;
	
	int getStride() const { return mStride;	}
	D3DPRIMITIVETYPE getPrimitiveType() const { return mPrimitiveType; }
	
private:
	const DWORD				mStride;
	const D3DPRIMITIVETYPE	mPrimitiveType;
};

// --------------------------------------------------------------------------

template< typename PARTICLE >
class CAbstractVBParticleRenderer : public CAbstractParticleRenderer<PARTICLE> {
public:
	enum { MAX_PARTICLES = 16000 }; // due to 16bit IBs
public:
	CAbstractVBParticleRenderer( int stride, D3DPRIMITIVETYPE primType )
		: CAbstractParticleRenderer<PARTICLE>(stride,primType) { }

	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	virtual CVBChunk::TSharedPtr end();

protected:
	int calcVertexCount( int particleCount ) const { return particleCount * 4; }

protected:
	CVBChunk::TSharedPtr	mChunk;
};


// --------------------------------------------------------------------------

/**
 *  Simple particle renderer (CParticle).
 */
template< typename PARTICLE >
class CVBParticleRenderer : public CAbstractVBParticleRenderer<PARTICLE> {
public:
	CVBParticleRenderer( float particleSize = 1.0f );
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	// IParticleRenderer
	virtual void render( TParticleVector const& particles );
protected:
	typedef SVertexXyzTex1	TVertex;
private:
	float					mParticleSize;
};

// --------------------------------------------------------------------------

/**
 *  Colored particle renderer (CColoredParticle).
 */
template< typename PARTICLE >
class CVBColoredParticleRenderer : public CAbstractVBParticleRenderer<PARTICLE> {
public:
	CVBColoredParticleRenderer( float particleSize = 1.0f );
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	// IParticleRenderer
	virtual void render( TParticleVector const& particles );
protected:
	typedef SVertexXyzDiffuseTex1	TVertex;
private:
	float					mParticleSize;
};


// --------------------------------------------------------------------------

/**
 *  Sized, rotated and colored particle renderer (CSizedRotColoredParticle).
 */
template< typename PARTICLE >
class CVBSizedRotColoredParticleRenderer : public CAbstractVBParticleRenderer<PARTICLE> {
public:
	CVBSizedRotColoredParticleRenderer();
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	// IParticleRenderer
	virtual void render( TParticleVector const& particles );
protected:
	typedef SVertexXyzDiffuseTex1	TVertex;
};


/**
 *  Animated, sized, rotated and colored particle renderer (CAnimatedSizedRotColoredParticle).
 */
template< typename PARTICLE >
class CVBAnimatedSizedRotColoredParticleRenderer : public CAbstractVBParticleRenderer<PARTICLE> {
public:
	CVBAnimatedSizedRotColoredParticleRenderer( int rows, int cols );
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	// IParticleRenderer
	virtual void render( TParticleVector const& particles );
protected:
	typedef SVertexXyzDiffuseTex1	TVertex;
	SVector2 mParticleTextureSize;
};


/**
 *  Animated blended, sized, rotated and colored particle renderer (CAnimatedSizedRotColoredParticle).
 */
template< typename PARTICLE >
class CVBAnimatedBlendedSizedRotColoredParticleRenderer : public CAbstractVBParticleRenderer<PARTICLE> {
public:
	CVBAnimatedBlendedSizedRotColoredParticleRenderer( int rows, int cols );
	virtual void begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount );
	// IParticleRenderer
	virtual void render( TParticleVector const& particles );
protected:
	typedef SVertexXyzDiffuseTex1	TVertex;
	SVector2 mParticleTextureSize;
};





// --------------------------------------------------------------------------
//  IMPLEMENTATION

#include "VBParticleRenderer_impl.h"


} // namespace

#endif
