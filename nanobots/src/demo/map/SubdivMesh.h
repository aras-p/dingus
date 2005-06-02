#ifndef __SUBDIV_MESH_H
#define __SUBDIV_MESH_H

#include "GenericMesh.h"


// --------------------------------------------------------------------------

/**
 *  A quad-based subdivision mesh.
 *
 *  Once the mesh is built, can perform uniform subdivision (each quad
 *  into four quads), and mesh smoothing (each vertex is placed at the average
 *  of it's neighbour vertices).
 */
class CSubdivMesh : public CGenericMesh<4,SVector3> {
public:
	/**
	 *  Uniformly subdivides the mesh.
	 *
	 *  Each face is subdivided into four faces, new vertices and edges
	 *  are created and topology information set up.
	 *
	 *  NOTE: the mesh must have valid half-edge information!
	 */
	void	subdivide();

	/**
	 *  For testing: build a simple cube, given it's corner coordinates.
	 */
	void	buildCube( const SVector3& vmin, const SVector3& vmax );
};



#endif
