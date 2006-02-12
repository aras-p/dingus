// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDERABLE_BUFFER_H
#define __RENDERABLE_BUFFER_H

#include "Renderable.h"
#include "../kernel/Proxies.h"
#include "../utils/MemoryPool.h"


namespace dingus {


class CRenderContext;


// --------------------------------------------------------------------------

/**
 *  Abstract base class for vertex and indexed vertex buffer renderables.
 */
class CAbstractRenderableBuffer : public CRenderable {
public:
	enum { VERTEX_STREAM_COUNT = 16 };
public:
	CAbstractRenderableBuffer( const SVector3* origin = 0, int priority = 0 );

	void setVB( CD3DVertexBuffer& vb, int stream = 0 );
	CD3DVertexBuffer const* getVB( int stream = 0 ) const { return mVB[stream]; }
	CD3DVertexBuffer* getVB( int stream = 0 ) { return mVB[stream]; }

	/** Resets VBs, strides, byte offsets and vertex decl. */
	void resetVBs();

	/** Set stream element size in bytes. */
	void setStride( unsigned int stride, int stream = 0 ) { mStride[stream] = stride; }
	/** Get stream element size in bytes. */
	unsigned int getStride( int stream = 0 ) const { return mStride[stream]; }

	/** Set stream offset in bytes. Can be not supported on pre-DX9 drivers. */
	void setByteOffset( unsigned int offset, int stream = 0 ) { mByteOffset[stream] = offset; }
	/** Get stream offset in bytes. */
	unsigned int getByteOffset( int stream = 0 ) const { return mByteOffset[stream]; }

	/** Set vertex declaration. If isn't set, you have to set FVF on the device. */
	void setVertexDecl( CD3DVertexDecl* decl ) { mVertexDecl = decl; }
	/** Get vertex declaration. */
	CD3DVertexDecl* getVertexDecl() const { return mVertexDecl; }

	/** Set primitive type to render. */
	void setPrimType( D3DPRIMITIVETYPE primitiveType ) { mPrimType = primitiveType; }
	/** Get primitive type to render. */
	D3DPRIMITIVETYPE getPrimType() const { return mPrimType; }

	/** Set primitive count to render. */
	void setPrimCount( int primitiveCount ) { mPrimCount = primitiveCount; }
	/** Get primitive count to render. */
	int getPrimCount() const { return mPrimCount; }

	virtual const CD3DVertexBuffer*	getUsedVB() const { return mVB[0]; }

protected:
	void	applyStreams();
	void	unapplyStreams();
	int		getLargestActiveStream() const { return mLargestActiveStream; }
	
private:
	CD3DVertexBuffer*	mVB[VERTEX_STREAM_COUNT];
	unsigned int		mStride[VERTEX_STREAM_COUNT];
	unsigned int		mByteOffset[VERTEX_STREAM_COUNT];
	int					mLargestActiveStream;
	CD3DVertexDecl*		mVertexDecl;

	D3DPRIMITIVETYPE	mPrimType;
	int					mPrimCount;
};


// --------------------------------------------------------------------------

/**
 *  Vertex buffer renderable.
 */
class CRenderableBuffer : public CAbstractRenderableBuffer {
public:
	CRenderableBuffer( const SVector3* origin = 0, int priority = 0 );

	/** Set vertex index to start rendering from. */
	void setStartVertex( int startVertex ) { mStartVertex = startVertex; }
	/** Get vertex index to start rendering from. */
	int getStartVertex() const { return mStartVertex; }
	
	virtual void render( const CRenderContext& ctx );
	virtual const CD3DIndexBuffer*	getUsedIB() const { return NULL; }
	
private:
	DECLARE_POOLED_ALLOC(dingus::CRenderableBuffer);
private:
	int	mStartVertex;
};


// --------------------------------------------------------------------------

/**
 *  Indexed vertex buffer renderable.
 */
class CRenderableIndexedBuffer : public CAbstractRenderableBuffer {
public:
	CRenderableIndexedBuffer( const SVector3* origin = 0, int priority = 0 );

	void setIB( CD3DIndexBuffer& ibuffer ) { mIB = &ibuffer; }
	const CD3DIndexBuffer* getIB() const { return mIB; }
	CD3DIndexBuffer* getIB() { return mIB; }

	/**
	 *  Set vertex that is treated as the first one. All indices will be relative
	 *  to this vertex. This can be done with setByteOffset() in many cases, but
	 *  sometimes it isn't supported.
	 */
	void setBaseVertex( int baseVertex ) { mBaseVertex = baseVertex; }
	/** Get vertex that is treated as the first one. */
	int getBaseVertex() const { return mBaseVertex; }

	/** Set the lowest vertex that will be referenced by IB. */
	void setMinVertex( int minVertex ) { mMinVertex = minVertex; }
	/** Get the lowest vertex that will be referenced by IB. */
	int getMinVertex() const { return mMinVertex; }

	/** Set the number of vertices from min vertex that will be referenced by IB. */
	void setNumVertices( int numVertices ) { mNumVertices = numVertices; }
	/** Get the number of vertices from min vertex that will be referenced by IB. */
	int getNumVertices() const { return mNumVertices; }

	/** Set the index from which to start rendering. */ 
	void setStartIndex( int startIndex ) { mStartIndex = startIndex; }
	/** Get the index from which to start rendering. */ 
	int getStartIndex() const { return mStartIndex; }

	virtual void render( const CRenderContext& ctx );
	virtual const CD3DIndexBuffer* getUsedIB() const { return mIB; }
	
private:
	DECLARE_POOLED_ALLOC(dingus::CRenderableIndexedBuffer);
private:
	CD3DIndexBuffer*	mIB;
	int		mBaseVertex;
	int		mMinVertex;
	int		mNumVertices;
	int		mStartIndex;
};



}; // namespace

#endif
