#ifndef __TRIANGLE_MESH_H
#define __TRIANGLE_MESH_H

#include "GenericMesh.h"
#include "SubdivMesh.h"

// --------------------------------------------------------------------------

/**
 *  A triangle-based mesh.
 */
class CTriangleMesh : public CGenericMesh<3,SVector3> {
public:
	void	initFromSubdivMesh( const CSubdivMesh& sm );
	void	optimize( float tolerance );
	/// @return false if hedge can't be collapsed
	bool	collapseHEdge( int hedge );

private:
	void	removeHEdge( int hedge, bool dontFixLast );
	void	removeHEdges( int count, int* hedges );
	void	removeVert( int vert );
	void	removeFace( int face, bool dontFixLast );
};



#endif
