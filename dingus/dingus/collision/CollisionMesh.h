// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __COLLISION_MESH_H
#define __COLLISION_MESH_H

#include <ode/ode.h>
#include "../math/Vector3.h"
#include "../utils/MemoryPool.h"


namespace dingus {


class CCollisionMesh {
public:
	CCollisionMesh( int nverts, int ntris, const SVector3* verts, int vertStride, const unsigned short* indices, bool indicesAreActuallyInts = false );
	~CCollisionMesh();

	/*
	int getVertexCount() const { return mVertexCount; }
	int getTriangleCount() const { return mTriangleCount; }
	const SVector3& getVertex( int index ) const { return mVertices[index]; }
	int getIndex( int index ) const { return mIndices[index]; }
	const int* getTriangle( int index ) const { return &mIndices[index*3]; }
	*/

	/*
	Opcode::Model& getOpcodeModel() { return mModel; }
	IndexedTriangle* getOpcodeTriangles() const { return (IndexedTriangle*)mIndices; }
	IceMaths::Point* getOpcodeVertices() const { return (IceMaths::Point*)mVertices; }
	*/

	dTriMeshDataID getTriMeshData() const { return mTriListData; }

private:
	DECLARE_POOLED_ALLOC(dingus::CCollisionMesh);
private:
	int*		mIndices;
	SVector3*	mVertices;
	int			mVertexCount;
	int			mTriangleCount;

	/** Pointer to data for ODE's dTriList. */
	dTriMeshDataID	mTriListData;
};

}; // namespace

#endif
