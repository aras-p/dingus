#ifndef __GENERIC_MESH_H
#define __GENERIC_MESH_H

#include <dingus/math/Vector3.h>


// --------------------------------------------------------------------------

/**
 *  A generic mesh structure for mesh operations.
 */
template< int N, typename vert_data >
class CGenericMesh {
public:
	enum { FACESIDES = N };
public:
	/**
	 *  Half edge structure.
	 */
	struct SHEdge {
		int		vert;	// vertex at the end of half-edge
		int		other;	// oppositely oriented half-edge
		int		face;	// face this half-edge belongs to
		int		next;	// next half-edge around the face
	};
	typedef std::vector<SHEdge>		THEdgeVector;
	

	/**
	 *  Vertex structure: position, normal, user data, half-edge.
	 */
	struct SVertex {
		SVector3	pos;
		SVector3	normal;
		vert_data	data;
		int			hedge;
	};
	typedef std::vector<SVertex>	TVertexVector;
	
	/**
	 *  Face structure: half-edge.
	 */
	struct SFace {
		int		hedge;
		int		verts[N];
	};
	typedef std::vector<SFace>		TFaceVector;

public:
	CGenericMesh() { };
	CGenericMesh( const CGenericMesh& r ) : mVerts(r.mVerts), mHEdges(r.mHEdges), mFaces(r.mFaces) { }
	void operator=( const CGenericMesh& r ) { mVerts = r.mVerts; mHEdges = r.mHEdges; mFaces = r.mFaces; }
	
	/**
	 *  Smooths the mesh.
	 *
	 *  Each vertex is placed into average position of it's neighboring vertices.
	 */
	void	smooth();

	/**
	 *  Fills full topology information from face vertex indices.
	 *
	 *  Each face has to have N valid vertex indices in defined order. This
	 *  method then creates all half-edges and other info.
	 */
	void	fillHalfEdges();

	/**
	 *  Recalculates face vertex indices from half-edge information.
	 */
	void	fillFaceVerts();

	/**
	 *  Computes normals of the vertices.
	 */
	void	computeNormals();

	/**
	 *  For testing: check validity of topology information. Asserts in
	 *  debug mode if the error is found.
	 */
	void	checkValidity() const;

	/// Get vertices.
	const TVertexVector& getVerts() const { return mVerts; }
	/// Get vertices.
	TVertexVector& getVerts() { return mVerts; }
	
	/// Get faces.
	const TFaceVector& getFaces() const { return mFaces; }
	/// Get faces.
	TFaceVector& getFaces() { return mFaces; }

	/// Get edges.
	const THEdgeVector& getHEdges() const { return mHEdges; }
	/// Get edges.
	THEdgeVector& getHEdges() { return mHEdges; }

	/// Clear the mesh.
	void	clear() { mVerts.clear(); mHEdges.clear(); mFaces.clear(); }

private:
	// Internal use
	struct SInternalSetHEdge {
		SInternalSetHEdge( int v1, int v2 ) {
			assert( v1 != v2 );
			if( v1 < v2 ) {
				verts[0] = v1; verts[1] = v2;
			} else {
				verts[0] = v2; verts[1] = v1;
			}
		}
		int		verts[2];
		int		index;
		bool	operator<( const SInternalSetHEdge& r ) const {
			if( verts[0] < r.verts[0] ) return true;
			if( verts[0] > r.verts[0] ) return false;
			if( verts[1] < r.verts[1] ) return true;
			return false;
		}
	};
	typedef std::set<SInternalSetHEdge>	THEdgeSet;

protected:
	TVertexVector	mVerts;
	THEdgeVector	mHEdges;
	TFaceVector		mFaces;
};


// --------------------------------------------------------------------------

template< int N, typename vert_data >
void CGenericMesh<N,vert_data>::checkValidity() const
{
	int nverts = mVerts.size();
	int nhedges = mHEdges.size();
	int nfaces = mFaces.size();
	int i;

	// go through verts
	for( i = 0; i < nverts; ++i ) {
		const SVertex& v = mVerts[i];
		// valid hedge index?
		assert( v.hedge >= 0 && v.hedge < nhedges );
		// go through adjacent edges, assure their pairs point to me
		int he = v.hedge;
		do {
			const SHEdge& h = mHEdges[mHEdges[he].other];
			assert( h.vert == i );
			he = h.next;
		} while( he != v.hedge );
	}

	// go through hedges
	for( i = 0; i < nhedges; ++i ) {
		const SHEdge& e = mHEdges[i];
		// valid vertex index?
		assert( e.vert >= 0 && e.vert < nverts );
		// valid face index?
		assert( e.face >= 0 && e.face < nfaces );
		// valid other hedge?
		assert( e.other >= 0 && e.other < nhedges );
		// other hedge paired with me?
		const SHEdge& ee = mHEdges[e.other];
		assert( ee.other == i );
		// edge pair are not from same vertex/face?
		assert( e.other != i );
		assert( e.vert != ee.vert );
		assert( e.face != ee.face );
		assert( e.next != ee.next );
	}

	// go through faces
	for( i = 0; i < nfaces; ++i ) {
		const SFace& f = mFaces[i];
		// valid hedge index?
		assert( f.hedge >= 0 && f.hedge < nhedges );
		// go through border hedges, check
		int count = 0;
		int he = f.hedge;
		do {
			const SHEdge& e = mHEdges[he];
			assert( e.face == i );
			he = e.next;
			++count;
		} while( he != f.hedge );
		assert( count == N );
	}
}

// --------------------------------------------------------------------------


template< int N, typename vert_data >
void CGenericMesh<N,vert_data>::smooth()
{
	// alloc array to hold new vertex positions
	int nverts = mVerts.size();
	int i;
	SVector3* newpos = new SVector3[nverts];

	// calculate new positions
	for( i = 0; i < nverts; ++i ) {
		const SVertex& v = mVerts[i];

		// poor man's smoothing: just average this and adjacent vertex positions :)
		int count = 0;
		SVector3 pos(0,0,0);
		int he = v.hedge;
		do {
			const SHEdge& h = mHEdges[he];
			const SVertex& vv = mVerts[h.vert];
			pos += vv.pos;
			he = mHEdges[h.other].next;
			++count;
		} while( he != v.hedge );
		pos *= (1.0f / count);
		newpos[i] = pos;
	}

	// replace old with new
	for( i = 0; i < nverts; ++i )
		mVerts[i].pos = newpos[i];
	delete[] newpos;
}



// --------------------------------------------------------------------------

template< int N, typename vert_data >
void CGenericMesh<N,vert_data>::fillHalfEdges()
{
	int i;

	int nverts = mVerts.size();
	int nfaces = mFaces.size();

	mHEdges.resize( nfaces * N );

	// clear hedges in verts
	for( i = 0; i < nverts; ++i ) {
		mVerts[i].hedge = -1;
	}
	// clear hedges in faces
	for( i = 0; i < nfaces; ++i ) {
		mFaces[i].hedge = -1;
	}

	//
	// go through faces, create hedges for them

	THEdgeSet hedgeSet;

	// create half-edges for faces
	for( i = 0; i < nfaces; ++i ) {
		SFace& f = mFaces[i];
		for( int j = 0; j < N; ++j ) {
			int ev0 = f.verts[j];
			int ev1 = f.verts[(j+1)%N];
			// create half-edge
			int heidx = i*N + j;
			SHEdge& he = mHEdges[heidx];
			he.vert = ev1;
			he.other = -1; // filled later
			he.face = i;
			he.next = i*N + (j+1)%N;
			// check if this vertex pair already occured
			SInternalSetHEdge setHe( ev0, ev1 );
			setHe.index = heidx;
			std::pair<THEdgeSet::iterator,bool> res = hedgeSet.insert( setHe );
			if( !res.second ) {
				// already occured - so we're the other half-edge
				SInternalSetHEdge& hh = *res.first;
				assert( hh.index < heidx );
				he.other = hh.index;
				assert( mHEdges[hh.index].other == -1 );
				assert( mHEdges[hh.index].vert == ev0 );
				mHEdges[hh.index].other = heidx;
			}
			// just assign this hedge to vert and face
			f.hedge = heidx;
			mVerts[ev0].hedge = heidx;
		}
	}

#ifdef _DEBUG
	// debug: check that all half-edges have pair indices
	for( i = 0; i < mHEdges.size(); ++i ) {
		const SHEdge& he = mHEdges[i];
		assert( he.other >= 0 );
		assert( mHEdges[he.other].other == i );
	}
#endif
}


// --------------------------------------------------------------------------

template< int N, typename vert_data >
void CGenericMesh<N,vert_data>::fillFaceVerts()
{
	int nfaces = mFaces.size();
	for( int i = 0; i < nfaces; ++i ) {
		SFace& f = mFaces[i];
		int j = 0;
		int heidx = f.hedge;
		do {
			f.verts[j] = mHEdges[heidx].vert;
			heidx = mHEdges[heidx].next;
			++j;
		} while( heidx != f.hedge );
	}
}


// --------------------------------------------------------------------------

template< int N, typename vert_data >
void CGenericMesh<N,vert_data>::computeNormals()
{
	int nverts = mVerts.size();

	int i;

	for( i = 0; i < nverts; ++i ) {
		SVertex& v = mVerts[i];
		v.normal.set(0,0,0);
		// go through vertex edges, calculate normals for their successive pairs
		int he1i = v.hedge;
		int he2i = mHEdges[mHEdges[he1i].other].next;
		do {
			const SHEdge& h1 = mHEdges[he1i];
			const SHEdge& h2 = mHEdges[he2i];
			const SVertex& v1 = mVerts[h1.vert];
			const SVertex& v2 = mVerts[h2.vert];
			SVector3 vec1 = v1.pos - v.pos;
			SVector3 vec2 = v2.pos - v.pos;
			v.normal += vec2.cross( vec1 );
			he1i = he2i;
			he2i = mHEdges[h2.other].next;
		} while( he1i != v.hedge );
		// normalize the normal
		v.normal.normalize();
	}
}



#endif
