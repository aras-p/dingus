// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderableBuffer.h"
#include "RenderContext.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;


DEFINE_POOLED_ALLOC(dingus::CRenderableBuffer,128,false);
DEFINE_POOLED_ALLOC(dingus::CRenderableIndexedBuffer,128,false);

// --------------------------------------------------------------------------
//  CAbstractRenderableBuffer
// --------------------------------------------------------------------------

CAbstractRenderableBuffer::CAbstractRenderableBuffer( const SVector3* origin, int priority )
:	CRenderable( origin, priority ),
	mPrimType( D3DPT_TRIANGLELIST ),
	mPrimCount( 0 )
{
	mLargestActiveStream = VERTEX_STREAM_COUNT - 1;
	resetVBs();
	mLargestActiveStream = -1;
}

void CAbstractRenderableBuffer::resetVBs()
{
	for( int q = 0; q <= mLargestActiveStream; ++q ) {
		mVB[q] = NULL;
		mStride[q] = 0;
		mByteOffset[q] = 0;
	}
	mLargestActiveStream = -1;
	mVertexDecl = NULL;
}

void CAbstractRenderableBuffer::setVB( CD3DVertexBuffer& vb, int stream )
{
	assert( stream >= 0 && stream < VERTEX_STREAM_COUNT );
	mVB[stream] = &vb;
	if( stream > mLargestActiveStream )
		mLargestActiveStream = stream;
}

void CAbstractRenderableBuffer::applyStreams()
{
	CD3DDevice& device = CD3DDevice::getInstance();

	for( int q = 0; q <= mLargestActiveStream; ++q ) {
		assert( mStride[q] > 0 );
		device.setVertexBuffer( q, mVB[q], mByteOffset[q], mStride[q] );
	}

	if( mVertexDecl )
		device.setDeclaration( *mVertexDecl );
}

void CAbstractRenderableBuffer::unapplyStreams()
{
	/*
	CD3DDevice& device = CD3DDevice::getInstance();
	for( int q = 0; q <= mLargestActiveStream; ++q )
		device.setVertexBuffer( q, NULL, 0, 0 );
		*/
}


// --------------------------------------------------------------------------
//  CRenderableBuffer
// --------------------------------------------------------------------------

CRenderableBuffer::CRenderableBuffer( const SVector3* origin, int priority )
:	CAbstractRenderableBuffer( origin, priority ),
	mStartVertex( 0 )
{
}


void CRenderableBuffer::render( CRenderContext const& ctx )
{
	if( getLargestActiveStream() < 0 )
		return;
	if( getPrimCount() == 0 )
		return;

	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();
	
	applyStreams();
	dx.DrawPrimitive( getPrimType(), mStartVertex, getPrimCount() );
	unapplyStreams();
	
	// stats
	stats.incDrawCalls();
	stats.incVerticesRendered( /* TBD */ getPrimCount() );
	stats.incPrimsRendered( getPrimCount() );
}


// --------------------------------------------------------------------------
//  CRenderableIndexedBuffer
// --------------------------------------------------------------------------

CRenderableIndexedBuffer::CRenderableIndexedBuffer( const SVector3* origin, int priority )
:	CAbstractRenderableBuffer( origin, priority ),
	mIB( NULL ),
	mBaseVertex( 0 ),
	mMinVertex( 0 ),
	mNumVertices( 0 ),
	mStartIndex( 0 )
{
}

void CRenderableIndexedBuffer::render( CRenderContext const& ctx )
{
	if( getLargestActiveStream() < 0 )
		return;
	if( getPrimCount() == 0 )
		return;

	assert( mIB );

	CD3DDevice& device = CD3DDevice::getInstance();
	IDirect3DDevice9& dx = device.getDevice();
	CRenderStats& stats = device.getStats();

	applyStreams();

	device.setIndexBuffer( mIB );

	dx.DrawIndexedPrimitive(
		getPrimType(), mBaseVertex, mMinVertex, mNumVertices, mStartIndex, getPrimCount() );

	unapplyStreams();

	//device.setIndexBuffer( NULL );

	// stats
	stats.incDrawCalls();
	stats.incVerticesRendered( mNumVertices );
	stats.incPrimsRendered( getPrimCount() );
}
