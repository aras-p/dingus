#include "stdafx.h"
#include "TriangleMesh.h"
#include <dingus/math/Plane.h>


// --------------------------------------------------------------------------

void CTriangleMesh::initFromSubdivMesh( const CSubdivMesh& sm )
{
	int i, n;

	// copy verts
	mVerts.resize( sm.getVerts().size() );
	n = sm.getVerts().size();
	for( i = 0; i < n; ++i ) {
		mVerts[i].pos = sm.getVerts()[i].pos;
		mVerts[i].normal = sm.getVerts()[i].normal;
		const SVector3& d = sm.getVerts()[i].data;
		mVerts[i].data.set( d.x, d.y, d.z, 0.0f );
	}

	// create faces - each quad into two tris
	mFaces.resize( sm.getFaces().size() * 2 );
	n = sm.getFaces().size();
	for( i = 0; i < n; ++i ) {
		const CSubdivMesh::SFace& f = sm.getFaces()[i];
		SFace& fa = mFaces[i*2+0];
		fa.verts[0] = f.verts[0];
		fa.verts[1] = f.verts[1];
		fa.verts[2] = f.verts[2];
		SFace& fb = mFaces[i*2+1];
		fb.verts[0] = f.verts[0];
		fb.verts[1] = f.verts[2];
		fb.verts[2] = f.verts[3];
	}

	// fill topology
	fillHalfEdges();
	//checkValidity();
}

void CTriangleMesh::optimize( float tolerance )
{
	int i;

	srand(0);

	int nverts = mVerts.size();
	for( i = 0; i < nverts; ++i )
		mVerts[i].data.w = 0.0f;

	// go thru hedges
	CONS << "optimize start: verts=" << (int)mVerts.size() << " tris=" << (int)mFaces.size() << endl;
	int collsDone = 0;

	for( int pass = 0; pass < 2; ++pass ) {
		int degenTriCount = 0;

		for( i = 0; i < mHEdges.size();  ) {
			const SHEdge& he = mHEdges[i];
			int vIdxTo = he.vert;
			int vIdxFrom = mHEdges[he.other].vert;
			SVertex& vto = mVerts[vIdxTo];
			const SVertex& vfrom = mVerts[vIdxFrom];

			// go through "from" vertex adjacent faces, project "to" vertex
			// onto them, calc error
			float hedgeError = vfrom.data.w + vto.data.w;
			// cheesy fix: if "from" vert is on 0.0 height, and "to" is not, OR
			// vert heights are of different signs
			// then try to prevent this edge collapse
			if( fabsf(vfrom.pos.y) < 0.1f && fabsf(vto.pos.y) > 0.1f )
				hedgeError = tolerance;
			else if( vfrom.pos.y * vto.pos.y < -0.1f )
				hedgeError = tolerance;


			
			// can we collapse the hedge?
			if( hedgeError >= tolerance ) {
				++i;
				continue;
			}

			int adjhe = vfrom.hedge;
			do {
				const SHEdge& he1 = mHEdges[adjhe];
				const SHEdge& he2 = mHEdges[he1.next];
				// just skip collapsing this
				if( he1.vert != vIdxTo && he2.vert != vIdxTo ) {
					// get verts
					const SVertex& vert1 = mVerts[he1.vert];
					const SVertex& vert2 = mVerts[he2.vert];
					// penalize long thin triangles
					SVector3 vecSide1 = vert1.pos - vto.pos;
					SVector3 vecSide2 = vert2.pos - vto.pos;
					vecSide1.normalize();
					vecSide2.normalize();
					float sideDot = vecSide1.dot( vecSide2 );
					const float SKINNY = 0.99f;
					const float VERYSKINNY = 0.998f;
					if( sideDot < -VERYSKINNY || sideDot > VERYSKINNY )
						hedgeError += tolerance;
					else if( sideDot < -SKINNY || sideDot > SKINNY )
						hedgeError += tolerance * 0.2f;
					else {
						// check if this tri is degenerate
						// TBD: why there are degenerates in the first place???
						vecSide1 = vert1.pos - vfrom.pos;
						vecSide2 = vert2.pos - vfrom.pos;
						vecSide1.normalize();
						vecSide2.normalize();
						sideDot = vecSide1.dot( vecSide2 );
						if( sideDot < -VERYSKINNY || sideDot > VERYSKINNY ) {
							hedgeError += tolerance;
							++degenTriCount;
						} else {
							// calc plane
							SPlane plane( vfrom.pos, vert1.pos, vert2.pos );
							/*
							if( plane.a < -100 || plane.a > 100 )
								assert( false );
							if( plane.b < -100 || plane.b > 100 )
								assert( false );
							if( plane.c < -100 || plane.c > 100 )
								assert( false );
							if( plane.d < -1000 || plane.d > 1000 )
								assert( false );*/
							// distance of "to" to plane - error
							float errPos = fabsf( plane.distance( vto.pos ) );
							//if( errPos < -1.0e6f || errPos > 1.0e6f )
							//	assert( false );
							hedgeError += errPos;
						}
					}
				}
				adjhe = mHEdges[he1.other].next;
			} while( adjhe != vfrom.hedge && hedgeError < tolerance );

			// can we collapse the hedge?
			if( hedgeError >= tolerance ) {
				++i;
				continue;
			}

			assert( hedgeError >= 0.0f );
			vto.data.w = hedgeError;
			bool ok = collapseHEdge( i );
			if( ok ) {
				++collsDone;
			} else {
				++i;
			}
		}
		CONS << "  degenerate tris: " << degenTriCount << endl;
		//CONS << "optimize pass " << pass << " collapses=" << collsDone << endl;
	}
	CONS << "optimize ended: verts=" << (int)mVerts.size() << " tris=" << (int)mFaces.size() << " collapses=" << collsDone << endl;
	
	fillFaceVerts();
	checkValidity();

	// DEBUG
	nverts = mVerts.size();
	int* vcounts = new int[nverts];
	memset( vcounts, 0, nverts*sizeof(vcounts[0]) );
	int nfaces = mFaces.size();
	for( i = 0; i < nfaces; ++i ) {
		const SFace& f = mFaces[i];
		++vcounts[f.verts[0]];
		++vcounts[f.verts[1]];
		++vcounts[f.verts[2]];
	}
	for( i = 0; i < nverts; ++i ) {
		if( vcounts[i] >= 16 ) {
			CONS << "vert " << i << " in " << vcounts[i] << " at " << mVerts[i].pos << endl;
		}
	}
	delete[] vcounts;
}

bool CTriangleMesh::collapseHEdge( int hedge )
{
	// collapse removes:
	//	1 vertex (starting vert of this hedge)
	//	2 tris (bordering this hedge pair)
	//	6 half-edges (from the removed two tris)
	int he1idx = hedge;		const SHEdge& he1 = mHEdges[he1idx];
	int he2idx = he1.other;	const SHEdge& he2 = mHEdges[he2idx];
	int vIdxFrom = he2.vert;
	int vIdxTo = he1.vert;

	int fidx1 = he1.face;
	int fidx2 = he2.face;

	int colHedge1a = he1.next;		const SHEdge& colHe1a = mHEdges[colHedge1a];
	int colHedge1b = colHe1a.next;	const SHEdge& colHe1b = mHEdges[colHedge1b];
	int colHedge2a = he2.next;		const SHEdge& colHe2a = mHEdges[colHedge2a];
	int colHedge2b = colHe2a.next;	const SHEdge& colHe2b = mHEdges[colHedge2b];

	// check if hedge can be collapsed
	// side verts the same - no collapse
	int vIdxSide1 = colHe1a.vert;
	int vIdxSide2 = colHe2a.vert;
	if( vIdxSide1 == vIdxSide2 )
		return false;
	const SVertex& vFrom = mVerts[vIdxFrom];
	SVertex& vTo = mVerts[vIdxTo];
	// if hedge vertices have common neighbors (excluding side verts),
	// then no collapse possible
	{
		int vhe1 = vFrom.hedge;
		do {
			const SHEdge& he = mHEdges[vhe1];
			int hvertTo1 = he.vert;

			if( hvertTo1 != vIdxSide1 && hvertTo1 != vIdxSide2 ) {
				int vhe2 = vTo.hedge;
				do {
					const SHEdge& hhe = mHEdges[vhe2];
					int hvertTo2 = hhe.vert;
					if( hvertTo2 == hvertTo1 )
						return false; // no collapse
					vhe2 = mHEdges[hhe.other].next;
				} while( vhe2 != vTo.hedge );
			}

			vhe1 = mHEdges[he.other].next;
		} while( vhe1 != vFrom.hedge );
	}

	// go through "from" vertex hedges and update them to point to "to" vertex
	{
		int vhe = vFrom.hedge;
		do {
			SHEdge& he = mHEdges[ mHEdges[vhe].other ];
			assert( he.vert == vIdxFrom );
			he.vert = vIdxTo;
			vhe = he.next;
		} while( vhe != vFrom.hedge );
	}

	// collapsing tris: pair their outer hedges
	mHEdges[colHe1a.other].other = colHe1b.other;
	mHEdges[colHe1b.other].other = colHe1a.other;
	mHEdges[colHe2a.other].other = colHe2b.other;
	mHEdges[colHe2b.other].other = colHe2a.other;
	// update the side verts' hedge indices
	mVerts[vIdxSide1].hedge = colHe1a.other;
	mVerts[vIdxSide2].hedge = colHe2a.other;
	vTo.hedge = colHe2b.other;

	// now, actually remove the things

	// hedges
	const int colHeCount = 6;
	int colHedges[colHeCount];
	colHedges[0] = he1idx;		colHedges[1] = he2idx;
	colHedges[2] = colHedge1a;	colHedges[3] = colHedge1b;
	colHedges[4] = colHedge2a;	colHedges[5] = colHedge2b;
	removeHEdges( colHeCount, colHedges );
	// faces
	removeFace( fidx1, (fidx2==mFaces.size()-1) );	if( fidx2 == mFaces.size() ) fidx2 = fidx1;
	removeFace( fidx2, false );
	// vertex
	removeVert( vIdxFrom );

	return true;
}

void CTriangleMesh::removeHEdges( int count, int* hedges )
{
	for( int i = 0; i < count; ++i ) {
		int j;
		bool last4remove = false;
		for( j = i; j < count; ++j ) {
			if( hedges[j] == mHEdges.size()-1 ) {
				last4remove = true;
				break;
			}
		}
		removeHEdge( hedges[i], last4remove );
		for( j = i+1; j < count; ++j ) {
			if( hedges[j] == mHEdges.size() )
				hedges[j] = hedges[i];
		}
	}
}

void CTriangleMesh::removeHEdge( int hedge, bool dontFixLast )
{
	// remove half-edge: place last one in place of it
	int lastHIdx = mHEdges.size()-1;
	SHEdge& he = mHEdges[hedge];
	he = mHEdges[lastHIdx];
	mHEdges.pop_back();

	assert( dontFixLast || (hedge!=lastHIdx) );

	if( dontFixLast )
		return;

	// update it's paired hedge index
	mHEdges[he.other].other = hedge;

	// update it's face hedge index
	mFaces[he.face].hedge = hedge;

	// update it's previous hedge to point to new index, and update the vertex hedge
	SHEdge* hhe = &mHEdges[he.next];
	do {
		hhe = &mHEdges[hhe->next];
	} while( hhe->next != lastHIdx );
	hhe->next = hedge;
	SVertex& v = mVerts[hhe->vert];
	if( v.hedge == lastHIdx )
		v.hedge = hedge;
}

void CTriangleMesh::removeVert( int vert )
{
	// remove vertex: place last one in place of it
	int lastVIdx = mVerts.size()-1;
	SVertex& v = mVerts[vert];
	v = mVerts[lastVIdx];
	mVerts.pop_back();
	if( vert == lastVIdx )
		return;

	// go through adjacent half-edges and update the vertex index
	int hedge = v.hedge;
	do {
		SHEdge& he = mHEdges[ mHEdges[hedge].other ];
		assert( he.vert == lastVIdx );
		he.vert = vert;
		hedge = he.next;
	} while( hedge != v.hedge );
}

void CTriangleMesh::removeFace( int face, bool dontFixLast )
{
	// remove face: place last one in place of it
	int lastFIdx = mFaces.size()-1;
	if( lastFIdx == face )
		dontFixLast = true;
	SFace& f = mFaces[face];
	f = mFaces[lastFIdx];
	mFaces.pop_back();

	if( dontFixLast )
		return;

	// go through border half-edges and update the face index
	int hedge = f.hedge;
	do {
		SHEdge& he = mHEdges[hedge];
		assert( he.face == lastFIdx );
		he.face = face;
		hedge = he.next;
	} while( hedge != f.hedge );
}


/*
void CTriangleMesh::collapseEdge( int vidx, int vedge )
{
	int i, j;

	const SVertex& cv = mVerts[vidx];
	int colEdgeIdx = cv.edges[vedge];
	const SEdge& cedge = mEdges[colEdgeIdx];

	int newVertIdx = cedge.getOtherVert( vidx );

	// collapse removes:
	//	1 vertex [i]
	//	2 faces [bordering edge colEdgeIdx]
	//	3 edges [colEdgeIdx, and two more that go from collapsed vertex and border the collapsed faces]
	for( i = 0; i < 2; ++i ) {
		// collapsing face
		int colFaceIdx = cedge.faces[i];
		const SFace& colFace = mFaces[colFaceIdx];
		// find collapsing edge
		int colEdgeSideIdx = -1;
		for( j = 0; j < cv.edgeCount; ++j ) {
			int ei = cv.edges[j];
			if( ei == colEdgeIdx ) continue;
			if( mEdges[ei].findFace( colFaceIdx ) >= 0 ) {
				colEdgeSideIdx = ei;
				break;
			}
		}
		assert( colEdgeSideIdx >= 0 );
		// find non-collapsed edge in the collapsing face
		// (one edge is the main collapse edge, other is "side edge")
		int noColEdgeIdx = -1;
		if( colFace.edges[0]!=colEdgeIdx && colFace.edges[0]!=colEdgeSideIdx ) noColEdgeIdx = colFace.edges[0];
		else if( colFace.edges[1]!=colEdgeIdx && colFace.edges[1]!=colEdgeSideIdx ) noColEdgeIdx = colFace.edges[1];
		else if( colFace.edges[2]!=colEdgeIdx && colFace.edges[2]!=colEdgeSideIdx ) noColEdgeIdx = colFace.edges[2];
		assert( noColEdgeIdx >= 0 );
		// now, update the topology:
		// collapsed face's non-collapsed edge:
		//   update it's one face index
		int sideFaceIdx = mEdges[colEdgeSideIdx].getOtherFace( colFaceIdx );
		SEdge& noColEdge = mEdges[noColEdgeIdx];
		int cfaceIdxNoColEdge = noColEdge.findFace( colFaceIdx );
		assert( cfaceIdxNoColEdge >= 0 );
		noColEdge.faces[cfaceIdxNoColEdge^1] = sideFaceIdx;
		//
	}
}
*/
