// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VB_PARTICLE_RENDERER_IMPL_H
#define __VB_PARTICLE_RENDERER_IMPL_H


// --------------------------------------------------------------------------
//  CAbstractVBParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
void CAbstractVBParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	static int warnCounter = 0;
	if( particleCount > MAX_PARTICLES ) {
		++warnCounter;
		if( !(warnCounter&63) )
			CConsole::CON_WARNING << "Too much particles: " << particleCount << endl;
		particleCount = MAX_PARTICLES;
	}
	mChunk = chunkSource.lock( calcVertexCount( particleCount ) );
}

template< typename PARTICLE >
CVBChunk::TSharedPtr CAbstractVBParticleRenderer<PARTICLE>::end()
{
	CVBChunk::TSharedPtr c = mChunk;
	mChunk->unlock();
	mChunk = CVBChunk::TSharedPtr();
	return c;
}

// --------------------------------------------------------------------------
//  CVBParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
CVBParticleRenderer<PARTICLE>::CVBParticleRenderer( float particleSize )
:	CAbstractVBParticleRenderer<PARTICLE>( sizeof(TVertex), D3DPT_TRIANGLELIST ),
	mParticleSize(particleSize)
{
}

template< typename PARTICLE >
void CVBParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	CAbstractVBParticleRenderer<PARTICLE>::begin( cameraPos, cameraRotMatrix, chunkSource, particleCount );
	CParticleRenderHelper::getInstance().begin( cameraRotMatrix, mParticleSize );
}

template< typename PARTICLE >
void CVBParticleRenderer<PARTICLE>::render( TParticleVector const& particles )
{
	assert( mChunk->getData() );
	TVertex* data = reinterpret_cast<TVertex*>( mChunk->getData() );

	CParticleRenderHelper const& helper = CParticleRenderHelper::getInstance();

	TParticleVector::const_iterator it, itEnd = particles.end();
	int i = 0;
	for( it = particles.begin(); it != itEnd; ++it, ++i ) {
		if( i > MAX_PARTICLES ) // don't render past max
			break;
		PARTICLE const& p = *it;

		data->p = p.getPosition() + helper.getTransformedCorner(0);
		data->tu = 0.0f;
		data->tv = 0.0f;
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(1);
		data->tu = 1.0f;
		data->tv = 0.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(2);
		data->tu = 1.0f;
		data->tv = 1.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(3);
		data->tu = 0.0f;
		data->tv = 1.0f;
		++data;
	}
}

// --------------------------------------------------------------------------
//  CVBColoredParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
CVBColoredParticleRenderer<PARTICLE>::CVBColoredParticleRenderer( float particleSize )
:	CAbstractVBParticleRenderer<PARTICLE>( sizeof(TVertex), D3DPT_TRIANGLELIST ),
	mParticleSize(particleSize)
{
}

template< typename PARTICLE >
void CVBColoredParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	CAbstractVBParticleRenderer<PARTICLE>::begin( cameraPos, cameraRotMatrix, chunkSource, particleCount );
	CParticleRenderHelper::getInstance().begin( cameraRotMatrix, mParticleSize );
}

template< typename PARTICLE >
void CVBColoredParticleRenderer<PARTICLE>::render( TParticleVector const& particles )
{
	assert( mChunk->getData() );
	TVertex* data = reinterpret_cast<TVertex*>( mChunk->getData() );

	CParticleRenderHelper const& helper = CParticleRenderHelper::getInstance();

	TParticleVector::const_iterator it, itEnd = particles.end();
	int i = 0;
	for( it = particles.begin(); it != itEnd; ++it, ++i ) {
		if( i > MAX_PARTICLES ) // don't render past max
			break;
		PARTICLE const& p = *it;

		data->p = p.getPosition() + helper.getTransformedCorner(0);
		data->diffuse = p.getColor();
		data->tu = 0.0f;
		data->tv = 0.0f;
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(1);
		data->diffuse = p.getColor();
		data->tu = 1.0f;
		data->tv = 0.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(2);
		data->diffuse = p.getColor();
		data->tu = 1.0f;
		data->tv = 1.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(3);
		data->diffuse = p.getColor();
		data->tu = 0.0f;
		data->tv = 1.0f;
		++data;
	}
}


// --------------------------------------------------------------------------
//  CVBSizedRotColoredParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
CVBSizedRotColoredParticleRenderer<PARTICLE>::CVBSizedRotColoredParticleRenderer()
:	CAbstractVBParticleRenderer<PARTICLE>( sizeof(TVertex), D3DPT_TRIANGLELIST )
{
}

template< typename PARTICLE >
void CVBSizedRotColoredParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	CAbstractVBParticleRenderer<PARTICLE>::begin( cameraPos, cameraRotMatrix, chunkSource, particleCount );
	CRotParticleRenderHelper::getInstance().begin( cameraRotMatrix );
}

template< typename PARTICLE >
void CVBSizedRotColoredParticleRenderer<PARTICLE>::render( TParticleVector const& particles )
{
	assert( mChunk->getData() );
	TVertex* data = reinterpret_cast<TVertex*>( mChunk->getData() );

	CRotParticleRenderHelper const& helper = CRotParticleRenderHelper::getInstance();

	TParticleVector::const_iterator it, itEnd = particles.end();
	int i = 0;
	for( it = particles.begin(); it != itEnd; ++it, ++i ) {
		if( i > MAX_PARTICLES ) // don't render past max
			break;
		PARTICLE const& p = *it;

		int rot = helper.getRotationIndex( p.getRotation() );
		float size = p.getSizeFraction() * p.getOrigSize();

		data->p = p.getPosition() + helper.getTransformedCorner(rot,0) * size;
		data->diffuse = p.getColor();
		data->tu = 0.0f;
		data->tv = 0.0f;
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(rot,1) * size;
		data->diffuse = p.getColor();
		data->tu = 1.0f;
		data->tv = 0.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,2) * size;
		data->diffuse = p.getColor();
		data->tu = 1.0f;
		data->tv = 1.0f;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,3) * size;
		data->diffuse = p.getColor();
		data->tu = 0.0f;
		data->tv = 1.0f;
		++data;
	}
}



// --------------------------------------------------------------------------
//  CVBAnimatedSizedRotColoredParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
CVBAnimatedSizedRotColoredParticleRenderer<PARTICLE>::CVBAnimatedSizedRotColoredParticleRenderer( int rows, int cols )
:	CAbstractVBParticleRenderer<PARTICLE>( sizeof(TVertex), D3DPT_TRIANGLELIST ),
	mParticleTextureSize( 1.0f / cols, 1.0f / rows )
{
}

template< typename PARTICLE >
void CVBAnimatedSizedRotColoredParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	CAbstractVBParticleRenderer<PARTICLE>::begin( cameraPos, cameraRotMatrix, chunkSource, particleCount );
	CRotParticleRenderHelper::getInstance().begin( cameraRotMatrix );
}

template< typename PARTICLE >
void CVBAnimatedSizedRotColoredParticleRenderer<PARTICLE>::render( TParticleVector const& particles )
{
	assert( mChunk->getData() );
	TVertex* data = reinterpret_cast<TVertex*>( mChunk->getData() );

	CRotParticleRenderHelper const& helper = CRotParticleRenderHelper::getInstance();

	TParticleVector::const_iterator it, itEnd = particles.end();
	int i = 0;
	for( it = particles.begin(); it != itEnd; ++it, ++i ) {
		if( i > MAX_PARTICLES ) // don't render past max
			break;
		PARTICLE const& p = *it;

		int rot = helper.getRotationIndex( p.getRotation() );
		float size = p.getSizeFraction() * p.getOrigSize();

		data->p = p.getPosition() + helper.getTransformedCorner(rot,0) * size;
		data->diffuse = p.getColor();
		data->tu = p.getTextureU();
		data->tv = p.getTextureV();
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(rot,1) * size;
		data->diffuse = p.getColor();
		data->tu = p.getTextureU() + mParticleTextureSize.x;
		data->tv = p.getTextureV();
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,2) * size;
		data->diffuse = p.getColor();
		data->tu = p.getTextureU() + mParticleTextureSize.x;
		data->tv = p.getTextureV() + mParticleTextureSize.y;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,3) * size;
		data->diffuse = p.getColor();
		data->tu = p.getTextureU();
		data->tv = p.getTextureV() + mParticleTextureSize.y;
		++data;
	}
}



// --------------------------------------------------------------------------
//  CVBAnimatedBlendedSizedRotColoredParticleRenderer
// --------------------------------------------------------------------------

template< typename PARTICLE >
CVBAnimatedBlendedSizedRotColoredParticleRenderer<PARTICLE>::CVBAnimatedBlendedSizedRotColoredParticleRenderer( int rows, int cols )
:	CAbstractVBParticleRenderer<PARTICLE>( sizeof(TVertex), D3DPT_TRIANGLELIST ),
	mParticleTextureSize( 1.0f / cols, 1.0f / rows )
{
}

template< typename PARTICLE >
void CVBAnimatedBlendedSizedRotColoredParticleRenderer<PARTICLE>::begin(
		const SVector3& cameraPos, const SMatrix4x4& cameraRotMatrix,
		IChunkSource<CVBChunk>& chunkSource, int particleCount )
{
	CAbstractVBParticleRenderer<PARTICLE>::begin( cameraPos, cameraRotMatrix, chunkSource, particleCount * 2 );
	CRotParticleRenderHelper::getInstance().begin( cameraRotMatrix );
}

template< typename PARTICLE >
void CVBAnimatedBlendedSizedRotColoredParticleRenderer<PARTICLE>::render( TParticleVector const& particles )
{
	assert( mChunk->getData() );
	TVertex* data = reinterpret_cast<TVertex*>( mChunk->getData() );

	CRotParticleRenderHelper const& helper = CRotParticleRenderHelper::getInstance();

	TParticleVector::const_iterator it, itEnd = particles.end();
	int i = 0;
	for( it = particles.begin(); it != itEnd; ++it, ++i ) {
		if( i > MAX_PARTICLES ) // don't render past max
			break;
		PARTICLE const& p = *it;

		int rot = helper.getRotationIndex( p.getRotation() );
		float size = p.getSizeFraction() * p.getOrigSize();

		// first layer
		data->p = p.getPosition() + helper.getTransformedCorner(rot,0) * size;
		data->diffuse = p.getColor1();
		data->tu = p.getTextureU1();
		data->tv = p.getTextureV1();
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(rot,1) * size;
		data->diffuse = p.getColor1();
		data->tu = p.getTextureU1() + mParticleTextureSize.x;
		data->tv = p.getTextureV1();
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,2) * size;
		data->diffuse = p.getColor1();
		data->tu = p.getTextureU1() + mParticleTextureSize.x;
		data->tv = p.getTextureV1() + mParticleTextureSize.y;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,3) * size;
		data->diffuse = p.getColor1();
		data->tu = p.getTextureU1();
		data->tv = p.getTextureV1() + mParticleTextureSize.y;
		++data;

		// second layer
		data->p = p.getPosition() + helper.getTransformedCorner(rot,0) * size;
		data->diffuse = p.getColor2();
		data->tu = p.getTextureU2();
		data->tv = p.getTextureV2();
		++data;
		
		data->p = p.getPosition() + helper.getTransformedCorner(rot,1) * size;
		data->diffuse = p.getColor2();
		data->tu = p.getTextureU2() + mParticleTextureSize.x;
		data->tv = p.getTextureV2();
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,2) * size;
		data->diffuse = p.getColor2();
		data->tu = p.getTextureU2() + mParticleTextureSize.x;
		data->tv = p.getTextureV2() + mParticleTextureSize.y;
		++data;

		data->p = p.getPosition() + helper.getTransformedCorner(rot,3) * size;
		data->diffuse = p.getColor2();
		data->tu = p.getTextureU2();
		data->tv = p.getTextureV2() + mParticleTextureSize.y;
		++data;
	}
}



#endif
