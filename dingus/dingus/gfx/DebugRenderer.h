// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DEBUG_RENDERER_H
#define __DEBUG_RENDERER_H

#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"
#include "geometry/VBChunk.h"
#include "Vertices.h"


namespace dingus {

class CRenderContext;
class CD3DXEffect;
class CRenderableBuffer;



class CDebugRenderer : public boost::noncopyable {
public:
	CDebugRenderer( CRenderContext& renderCtx, CD3DXEffect& effect );
	~CDebugRenderer();

	void	beginDebug();
	void	endDebug();

	void	renderLine( const SVector3& pta, const SVector3& ptb, float width, D3DCOLOR color );
	void	renderSphere( const SVector3& center, float radius, D3DCOLOR color );
	void	renderBox( const SMatrix4x4& matrix, const SVector3& size, D3DCOLOR color );
	void	renderCoordFrame( const SMatrix4x4& matrix, float size, D3DCOLOR color );
	void	renderQuad( const SVector3& pt0, const SVector3& pt1, const SVector3& pt2, const SVector3& pt3, D3DCOLOR color );
	void	renderTri( const SVector3& pt0, const SVector3& pt1, const SVector3& pt2, D3DCOLOR color );
	
	void	renderTris( const void* verts, int vstride, const int* indices, int ntris, D3DCOLOR color );
	void	renderTrisNMat( const void* verts, int vstride, const int* indices, int ntris, D3DCOLOR color, const SMatrix4x4& world );
	
private:
	typedef SVertexXyzNormalDiffuse	TDebugVertex;

	TDebugVertex*	requestVertices( int triangleCount );

	void	commitDebugGeometry();
	void	getNewChunk();

	void	internalRenderLine( const SVector3& pta, const SVector3& ptb, float width, D3DCOLOR color, TDebugVertex* vb ) const;
	static TDebugVertex*	internalRenderPatch( const SVector3& center,
		const SVector3& pta, const SVector3& ptb, const SVector3& ptc,
		float radius, D3DCOLOR color, TDebugVertex* vb, int level );

private:
	CRenderContext*		mRenderCtx;
	CRenderableBuffer*	mRenderableBuffer;

	CVBChunk::TSharedPtr	mVBChunk;
	int		mUsedVertsInChunk;
};

}; // namespace


#endif
