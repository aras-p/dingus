// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "CollisionMesh.h"


using namespace dingus;


DEFINE_POOLED_ALLOC(dingus::CCollisionMesh,64,false);


CCollisionMesh::CCollisionMesh( int nverts, int ntris, const SVector3* verts, int vertStride, const unsigned short* indices, bool indicesAreActuallyInts )
:	mVertexCount(nverts),
	mTriangleCount(ntris)
{
	assert( nverts > 0 );
	assert( ntris > 0 );
	assert( verts );
	assert( vertStride >= sizeof(SVector3) );
	assert( indices );
	
	//
	// create vertices/indices

	mIndices = new int[ ntris * 3 ];
	assert( mIndices );
	mVertices = new SVector3[ mVertexCount ];
	assert( mVertices );

	//
	// copy data

	int i;
	for( i = 0; i < nverts; ++i ) {
		mVertices[i] = *verts;
		verts = (const SVector3*)( (char*)verts + vertStride );
	}
	if( !indicesAreActuallyInts ) {
		for( i = 0; i < ntris*3; ++i )
			mIndices[i] = indices[i];
	} else {
		const int* indices32 = (const int*)indices;
		for( i = 0; i < ntris*3; ++i )
			mIndices[i] = indices32[i];
	}

	//
	// build ODE trilist

	mTriListData = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle( mTriListData,
		mVertices, sizeof(SVector3), mVertexCount,
		mIndices, mTriangleCount*3, 3*sizeof(int) );
}


CCollisionMesh::~CCollisionMesh()
{
	assert( mTriListData );
		dGeomTriMeshDataDestroy( mTriListData );
	assert( mVertices );
	delete[] mVertices;
	assert( mIndices );
	delete[] mIndices;
}
