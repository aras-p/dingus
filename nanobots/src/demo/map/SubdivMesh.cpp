#include "stdafx.h"
#include "SubdivMesh.h"


// --------------------------------------------------------------------------

void CSubdivMesh::subdivide()
{
	CSubdivMesh dst;

	int nverts = mVerts.size();
	int nhedges = mHEdges.size();
	int nfaces = mFaces.size();
	int i;

	int resNumVerts = nverts + nhedges/2 + nfaces;
	int resNumFaces = nfaces*4;

	dst.mVerts.resize( resNumVerts );
	dst.mFaces.resize( resNumFaces );

	int* vertIdxForHedge = new int[nhedges];
	memset( vertIdxForHedge, -1, nhedges*sizeof(vertIdxForHedge[0]) );

	// DEBUG
	//memset( &dst.mVerts[0], 0xDDDDDDDD, resNumVerts*sizeof(dst.mVerts[0]) );
	//memset( &dst.mFaces[0], 0xDDDDDDDD, resNumFaces*sizeof(dst.mFaces[0]) );
	// /DEBUG

	//
	// create verts

	// 1. old verts
	for( i = 0; i < nverts; ++i ) {
		const SVertex& vs = mVerts[i];
		SVertex& vd = dst.mVerts[i];
		vd.pos = vs.pos;
		vd.data = vs.data;
	}
	// 2. new verts at edge midpoints
	int verts4hedges = nverts;
	for( i = 0; i < nhedges; ++i ) {
		const SHEdge& he = mHEdges[i];
		// only create vertex for one half-edge in pair: the one with lower index
		if( he.other < i ) continue;
		SVertex& vd = dst.mVerts[verts4hedges];
		const SVertex& v1 = mVerts[he.vert];
		const SVertex& v2 = mVerts[mHEdges[he.other].vert];
		vd.pos = (v1.pos + v2.pos) * 0.5f;
		vd.data = (v1.data + v2.data) * 0.5f;
		vertIdxForHedge[i] = verts4hedges;
		++verts4hedges;
	}
	assert( verts4hedges == nverts + nhedges/2 );
	// 3. new verts at face midpoints
	for( i = 0; i < nfaces; ++i ) {
		const SFace& f = mFaces[i];
		SVertex& vd = dst.mVerts[verts4hedges + i];
		vd.pos = mVerts[f.verts[0]].pos;
		vd.data = mVerts[f.verts[0]].data;
		for( int j = 1; j < FACESIDES; ++j ) {
			const SVertex& vs = mVerts[f.verts[j]];
			vd.pos += vs.pos;
			vd.data += vs.data;
		}
		float mul = (1.0f / FACESIDES);
		vd.pos *= mul;
		vd.data *= mul;
	}

	//
	// create faces

	// 1. each face into 4 faces
	for( i = 0; i < nfaces; ++i ) {
		const SFace& f = mFaces[i];
		int facevert = verts4hedges + i;
		int faceVerts[FACESIDES];
		int faceEdgeVerts[FACESIDES];

		int j = 0;
		int heidx = f.hedge;
		do {
			const SHEdge& he = mHEdges[heidx];
			int vertHe = heidx;
			if( he.other < heidx ) vertHe = he.other;
			faceVerts[j] = he.vert;
			faceEdgeVerts[j] = vertIdxForHedge[vertHe];
			assert( faceEdgeVerts[j] >= nverts );
			heidx = he.next;
			++j;
		} while( heidx != f.hedge );
		assert( j == FACESIDES );

		for( j = 0; j < FACESIDES; ++j ) {
			int fdi = i*4 + j;
			SFace& fd = dst.mFaces[fdi];
			fd.verts[0] = faceVerts[j];
			fd.verts[1] = faceEdgeVerts[(j+1)&3];
			fd.verts[2] = facevert;
			fd.verts[3] = faceEdgeVerts[j];
		}
	}

	delete[] vertIdxForHedge;

	// assign into self
	*this = dst;

	// fill half-edges
	fillHalfEdges();

	checkValidity();
}

// --------------------------------------------------------------------------

void CSubdivMesh::buildCube( const SVector3& vmin, const SVector3& vmax )
{
	clear();
	mVerts.resize(8);
	mFaces.resize(6);

	mVerts[0].pos.set( vmin.x, vmin.y, vmin.z );
	mVerts[1].pos.set( vmin.x, vmin.y, vmax.z );
	mVerts[2].pos.set( vmin.x, vmax.y, vmin.z );
	mVerts[3].pos.set( vmin.x, vmax.y, vmax.z );
	mVerts[4].pos.set( vmax.x, vmin.y, vmin.z );
	mVerts[5].pos.set( vmax.x, vmin.y, vmax.z );
	mVerts[6].pos.set( vmax.x, vmax.y, vmin.z );
	mVerts[7].pos.set( vmax.x, vmax.y, vmax.z );

	mFaces[0].verts[0] = 0; mFaces[0].verts[1] = 2; mFaces[0].verts[2] = 3; mFaces[0].verts[3] = 1;
	mFaces[1].verts[0] = 1; mFaces[1].verts[1] = 3; mFaces[1].verts[2] = 7; mFaces[1].verts[3] = 5;
	mFaces[2].verts[0] = 5; mFaces[2].verts[1] = 7; mFaces[2].verts[2] = 6; mFaces[2].verts[3] = 4;
	mFaces[3].verts[0] = 4; mFaces[3].verts[1] = 6; mFaces[3].verts[2] = 2; mFaces[3].verts[3] = 0;
	mFaces[4].verts[0] = 2; mFaces[4].verts[1] = 6; mFaces[4].verts[2] = 7; mFaces[4].verts[3] = 3;
	mFaces[5].verts[0] = 5; mFaces[5].verts[1] = 4; mFaces[5].verts[2] = 0; mFaces[5].verts[3] = 1;

	fillHalfEdges();
}
