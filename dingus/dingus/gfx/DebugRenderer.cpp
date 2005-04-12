// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "DebugRenderer.h"
#include "../renderer/RenderContext.h"
#include "../renderer/RenderableBuffer.h"
#include "geometry/VBManagerSource.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;

const int DEBUG_CHUNK_SIZE = 2048;

const DWORD DEBUG_FVF = FVF_XYZ_NORMAL_DIFFUSE;


CDebugRenderer::CDebugRenderer( CRenderContext& renderCtx, CD3DXEffect& effect )
:	mRenderCtx(&renderCtx), mVBChunk(0), mUsedVertsInChunk(0)
{
	assert( mRenderCtx );
	mRenderableBuffer = new CRenderableBuffer( NULL, 0 );
	mRenderableBuffer->getParams().setEffect( effect );
}

CDebugRenderer::~CDebugRenderer()
{
	assert( mRenderableBuffer );
	delete mRenderableBuffer;
}

void CDebugRenderer::beginDebug()
{
	mRenderCtx->directBegin();
}

void CDebugRenderer::endDebug()
{
	commitDebugGeometry();
	mRenderCtx->directEnd();
}

void CDebugRenderer::commitDebugGeometry()
{
	// nothing to render
	if( !mVBChunk || !mUsedVertsInChunk )
		return;

	// render used chunk portion
	CVBChunk& chunk = *mVBChunk;
	chunk.unlock( mUsedVertsInChunk );
	assert( mRenderableBuffer );
	mRenderableBuffer->resetVBs();
	mRenderableBuffer->setVB( chunk.getVB(), 0 );
	mRenderableBuffer->setStride( chunk.getStride(), 0 );
	mRenderableBuffer->setPrimType( D3DPT_TRIANGLELIST );
	assert( mUsedVertsInChunk % 3 == 0 );
	mRenderableBuffer->setPrimCount( mUsedVertsInChunk / 3 );
	mRenderableBuffer->setStartVertex( chunk.getOffset() );
	CD3DDevice& dx = CD3DDevice::getInstance();
	dx.setDeclarationFVF( DEBUG_FVF );
	mRenderCtx->directRender( *mRenderableBuffer );

	// reset used chunk
	mVBChunk = 0;
}

void CDebugRenderer::getNewChunk()
{
	CVBManagerSource chunkSource( sizeof(TDebugVertex) );
	mVBChunk = chunkSource.lock( DEBUG_CHUNK_SIZE );
	mUsedVertsInChunk = 0;
}

CDebugRenderer::TDebugVertex* CDebugRenderer::requestVertices( int triangleCount )
{
	assert( triangleCount > 0 );
	int vertexCount = triangleCount * 3;
	assert( vertexCount <= DEBUG_CHUNK_SIZE );

	// we might not have a chunk yet
	if( !mVBChunk || !mVBChunk->isValid() )
		getNewChunk();

	// if we don't have space in chunk - render it and get new one
	if( DEBUG_CHUNK_SIZE-mUsedVertsInChunk < vertexCount ) {
		commitDebugGeometry();
		getNewChunk();
	}
	
	TDebugVertex* ptr = (TDebugVertex*)mVBChunk->getData();
	ptr += mUsedVertsInChunk;
	mUsedVertsInChunk += vertexCount;
	return ptr;
}

void CDebugRenderer::internalRenderLine( const SVector3& pta, const SVector3& ptb, float width, D3DCOLOR color, TDebugVertex* vb ) const
{
	const SMatrix4x4& camRotMat = mRenderCtx->getCamera().getCameraRotMatrix();
	const SMatrix4x4& camViewMat = mRenderCtx->getCamera().getViewMatrix();
	const SVector3& camPos = mRenderCtx->getCamera().getEye3();

	// direction of the line in world space
	SVector3 segDir = ptb - pta;
	// get direction of "side" of the line, so that it's perpendicular to
	// line direction and view vector
	SVector3 side = segDir.cross( ptb - camPos );
	side.normalize();
	side *= width * 0.5f;
	SVector3 c1 = side;
	SVector3 c2 = -side;

	// normals
	SVector3 normal = segDir.cross( side );
	normal.normalize();

	// vertices
	// 1st triangle
	vb->p = pta - side;	vb->n = normal; vb->diffuse = color; ++vb;
	vb->p = pta + side; vb->n = normal; vb->diffuse = color; ++vb;
	vb->p = ptb + side; vb->n = normal; vb->diffuse = color; ++vb;
	// 2nd triangle
	vb->p = pta - side;	vb->n = normal; vb->diffuse = color; ++vb;
	vb->p = ptb + side; vb->n = normal; vb->diffuse = color; ++vb;
	vb->p = ptb - side; vb->n = normal; vb->diffuse = color; ++vb;
}

void CDebugRenderer::renderLine( const SVector3& pta, const SVector3& ptb, float width, D3DCOLOR color )
{
	TDebugVertex* vb = requestVertices( 2 );
	internalRenderLine( pta, ptb, width, color, vb );
}

void CDebugRenderer::renderCoordFrame( const SMatrix4x4& matrix, float size, D3DCOLOR color )
{
	float width = size * 0.1f;
	const SVector3& o = matrix.getOrigin();
	TDebugVertex* vb = requestVertices( 6 );

	internalRenderLine( o, o + matrix.getAxisX()*size, width, color & 0xFFff0000, vb +  0 );
	internalRenderLine( o, o + matrix.getAxisY()*size, width, color & 0xFF00ff00, vb +  6 );
	internalRenderLine( o, o + matrix.getAxisZ()*size, width, color & 0xFF0000ff, vb + 12 );
}

void CDebugRenderer::renderBox( const SMatrix4x4& matrix, const SVector3& size, D3DCOLOR color )
{
	const SVector3& o = matrix.getOrigin();
	SVector3 ax = matrix.getAxisX() * (size.x * 0.5f);
	SVector3 ay = matrix.getAxisY() * (size.y * 0.5f);
	SVector3 az = matrix.getAxisZ() * (size.z * 0.5f);
	SVector3 nx = matrix.getAxisX().getNormalized();
	SVector3 ny = matrix.getAxisY().getNormalized();
	SVector3 nz = matrix.getAxisZ().getNormalized();

	TDebugVertex* vb = requestVertices( 6*2 );
	
	// PX face
	vb->p = o + ax + ay + az; vb->n =  nx; vb->diffuse = color; ++vb;
	vb->p = o + ax + ay - az; vb->n =  nx; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay + az; vb->n =  nx; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay + az; vb->n =  nx; vb->diffuse = color; ++vb;
	vb->p = o + ax + ay - az; vb->n =  nx; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay - az; vb->n =  nx; vb->diffuse = color; ++vb;
	// NX face
	vb->p = o - ax + ay - az; vb->n = -nx; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay + az; vb->n = -nx; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay + az; vb->n = -nx; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay + az; vb->n = -nx; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay - az; vb->n = -nx; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay - az; vb->n = -nx; vb->diffuse = color; ++vb;
	// PY face
	vb->p = o + ax + ay - az; vb->n =  ny; vb->diffuse = color; ++vb;
	vb->p = o + ax + ay + az; vb->n =  ny; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay + az; vb->n =  ny; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay + az; vb->n =  ny; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay - az; vb->n =  ny; vb->diffuse = color; ++vb;
	vb->p = o + ax + ay - az; vb->n =  ny; vb->diffuse = color; ++vb;
	// NY face
	vb->p = o + ax - ay + az; vb->n = -ny; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay - az; vb->n = -ny; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay + az; vb->n = -ny; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay + az; vb->n = -ny; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay - az; vb->n = -ny; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay - az; vb->n = -ny; vb->diffuse = color; ++vb;
	// PZ face
	vb->p = o + ax + ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay + az; vb->n =  nz; vb->diffuse = color; ++vb;
	// NZ face
	vb->p = o + ax - ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
	vb->p = o + ax + ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
	vb->p = o - ax + ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
	vb->p = o - ax - ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
	vb->p = o + ax - ay - az; vb->n = -nz; vb->diffuse = color; ++vb;
}


// Recursively subdivides a triangular area (vertices pta,ptb,ptc) into
// smaller triangles, and then draws the triangles. All triangle vertices are
// normalized to a distance of 1.0 from the origin (pta,ptb,ptc are assumed
// to be already normalized).
CDebugRenderer::TDebugVertex* CDebugRenderer::internalRenderPatch(
		const SVector3& center,
		const SVector3& pta, const SVector3& ptb, const SVector3& ptc,
		float radius, D3DCOLOR color, TDebugVertex* vb, int level )
{
	if( level > 0 ) {
		// sub-vertices
		SVector3 q1 = (pta+ptb) * 0.5f;
		SVector3 q2 = (ptb+ptc) * 0.5f;
		SVector3 q3 = (ptc+pta) * 0.5f;
		// normalize them
		q1.normalize();
		q2.normalize();
		q3.normalize();
		// recurse
		--level;
		vb = internalRenderPatch( center, pta,  q1,  q3, radius, color, vb, level );
		vb = internalRenderPatch( center,  q1, ptb,  q2, radius, color, vb, level );
		vb = internalRenderPatch( center,  q1,  q2,  q3, radius, color, vb, level );
		vb = internalRenderPatch( center,  q3,  q2, ptc, radius, color, vb, level );
	} else {
		// do triangle
		vb->p = center + pta*radius; vb->n = pta; vb->diffuse = color; ++vb;
		vb->p = center + ptb*radius; vb->n = ptb; vb->diffuse = color; ++vb;
		vb->p = center + ptc*radius; vb->n = ptc; vb->diffuse = color; ++vb;
	}
	return vb;
}


void CDebugRenderer::renderSphere( const SVector3& center, float radius, D3DCOLOR color )
{
	// draws tesselated icosahedron
	const int TESS_LEVEL = 1;
	TDebugVertex* vb = requestVertices( 20 * 4 ); // depends on TESS_LEVEL!

	const float ICX = 0.525731112119133606f;
	const float ICZ = 0.850650808352039932f;
	static SVector3 ICO_DATA[12] = {
		SVector3(-ICX,    0,  ICZ),
		SVector3( ICX,    0,  ICZ),
		SVector3(-ICX,    0, -ICZ),
		SVector3( ICX,    0, -ICZ),
		SVector3(   0,  ICZ,  ICX),
		SVector3(   0,  ICZ, -ICX),
		SVector3(   0, -ICZ,  ICX),
		SVector3(   0, -ICZ, -ICX),
		SVector3( ICZ,  ICX,    0),
		SVector3(-ICZ,  ICX,    0),
		SVector3( ICZ, -ICX,    0),
		SVector3(-ICZ, -ICX,    0)
	};
	static int ICO_INDEX[20][3] = {
		{0, 4, 1},	{0, 9, 4},
		{9, 5, 4},	{4, 5, 8},
		{4, 8, 1},	{8, 10, 1},
		{8, 3, 10},	{5, 3, 8},
		{5, 2, 3},	{2, 7, 3},
		{7, 10, 3},	{7, 6, 10},
		{7, 11, 6},	{11, 0, 6},
		{0, 1, 6},	{6, 1, 10},
		{9, 0, 11},	{9, 11, 2},
		{9, 2, 5},	{7, 2, 11},
	};
    for( int i = 0; i < 20; ++i ) {
		// draw patch
		vb = internalRenderPatch( center,
			ICO_DATA[ICO_INDEX[i][0]],
			ICO_DATA[ICO_INDEX[i][1]],
			ICO_DATA[ICO_INDEX[i][2]],
			radius, color, vb, TESS_LEVEL );
	}
}

void CDebugRenderer::renderQuad( const SVector3& pt0, const SVector3& pt1, const SVector3& pt2, const SVector3& pt3, D3DCOLOR color )
{
	renderTri( pt0, pt1, pt2, color );
	renderTri( pt0, pt2, pt3, color );
}

void CDebugRenderer::renderTri( const SVector3& pt0, const SVector3& pt1, const SVector3& pt2, D3DCOLOR color )
{
	TDebugVertex* vb = requestVertices( 1 );
	SVector3 e01 = pt1 - pt0;
	SVector3 e02 = pt2 - pt0;
	SVector3 n = e01.cross(e02).getNormalized();
	vb->p = pt0; vb->n = n; vb->diffuse = color; ++vb;
	vb->p = pt1; vb->n = n; vb->diffuse = color; ++vb;
	vb->p = pt2; vb->n = n; vb->diffuse = color; ++vb;
}

void CDebugRenderer::renderTris( const void* verts, int vstride, const int* indices, int ntris, D3DCOLOR color )
{
	const char* vb = (const char*)verts;
	for( int i = 0; i < ntris; ++i ) {
		const SVector3* pt0 = (const SVector3*)(vb + indices[0]*vstride);
		const SVector3* pt1 = (const SVector3*)(vb + indices[1]*vstride);
		const SVector3* pt2 = (const SVector3*)(vb + indices[2]*vstride);
		renderTri( *pt0, *pt1, *pt2, color );
		indices += 3;
	}
}

void CDebugRenderer::renderTrisNMat( const void* verts, int vstride, const int* indices, int ntris, D3DCOLOR color, const SMatrix4x4& world )
{
	TDebugVertex* vb = requestVertices( ntris );

	const char* vertsByte = (const char*)verts;
	for( int i = 0; i < ntris; ++i ) {
		const SVertexXyzNormal* pt0 = (const SVertexXyzNormal*)(vertsByte + indices[0]*vstride);
		const SVertexXyzNormal* pt1 = (const SVertexXyzNormal*)(vertsByte + indices[1]*vstride);
		const SVertexXyzNormal* pt2 = (const SVertexXyzNormal*)(vertsByte + indices[2]*vstride);
		D3DXVec3TransformCoord( &vb->p, &pt0->p, &world );
		D3DXVec3TransformNormal( &vb->n, &pt0->n, &world );
		vb->diffuse = color;
		++vb;
		D3DXVec3TransformCoord( &vb->p, &pt1->p, &world );
		D3DXVec3TransformNormal( &vb->n, &pt1->n, &world );
		vb->diffuse = color;
		++vb;
		D3DXVec3TransformCoord( &vb->p, &pt2->p, &world );
		D3DXVec3TransformNormal( &vb->n, &pt2->n, &world );
		vb->diffuse = color;
		++vb;

		indices += 3;
	}
}

