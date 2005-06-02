#include "stdafx.h"

#include "Collider.h"
#include <opcode/Opcode.h>

// --------------------------------------------------------------------------

CCollisionMesh::CCollisionMesh( int nverts, int ntris, const SVector3* verts, int vertStride, const unsigned short* indices, bool indicesAreActuallyInts )
:	mVertexCount(nverts),
	mTriangleCount(ntris)
{
	assert( nverts > 0 );
	assert( ntris > 0 );
	assert( verts );
	assert( vertStride >= sizeof(SVector3) );
	assert( indices );
	
	//mMatrix.Identity();

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
	// build OPCODE model

	Opcode::OPCODECREATE opcc;
	opcc.mIMesh = new Opcode::MeshInterface();
	opcc.mIMesh->SetNbTriangles( mTriangleCount );
	opcc.mIMesh->SetNbVertices( mVertexCount );
	opcc.mIMesh->SetPointers( getOpcodeTriangles(), getOpcodeVertices() );
	// Tree building settings
	opcc.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
	opcc.mNoLeaf = true;
	opcc.mQuantized = false;
	opcc.mKeepOriginal = false; // true for debug purposes
	opcc.mCanRemap = true;
	bool s = mModel.Build( opcc ); // build the model
	assert( s );
}

CCollisionMesh::~CCollisionMesh()
{
	assert( mModel.GetMeshInterface() );
	delete mModel.GetMeshInterface();

	assert( mVertices );
	delete[] mVertices;
	assert( mIndices );
	delete[] mIndices;
}


// --------------------------------------------------------------------------

void CCollisionMesh::getTriangle( int triIdx, SVector3& v0, SVector3& v1, SVector3& v2, SVector3& normal ) const
{
	const IndexedTriangle& tri = getOpcodeTriangles()[triIdx];
	int i0 = tri.mVRef[0], i1 = tri.mVRef[1], i2 = tri.mVRef[2];

	const SVector3& mv0 = getVertex( i0 );
	const SVector3& mv1 = getVertex( i1 );
	const SVector3& mv2 = getVertex( i2 );

	v0 = mv0;
	v1 = mv1;
	v2 = mv2;
	
 	SVector3 v101 = v1 - v0;
 	SVector3 v102 = v2 - v0;
	normal = v101.cross( v102 ).getNormalized();
}

// --------------------------------------------------------------------------

CCollisionRay::CCollisionRay( const SVector3& position, const SVector3& direction )
:	mLength(MAX_FLOAT)
{
	setPosition( position );
	setDirection( direction );
}

void CCollisionRay::setDirection( const SVector3& direction ) 
{
	SVector3 d = direction;
	d.normalize();
	mRay.mDir = convert( d ); 
}

// --------------------------------------------------------------------------

CCollisionSphere::CCollisionSphere( float radius )
:	mSphere()
{
	//mMatrix.Identity();
	mSphere.SetRadius( radius );
	mSphere.mCenter.x = 0;
	mSphere.mCenter.y = 0;
	mSphere.mCenter.z = 0;
}

// --------------------------------------------------------------------------

CMeshRayCollider::CMeshRayCollider()
{
	mMeshRayCollider.SetFirstContact( false );
	mMeshRayCollider.SetTemporalCoherence( false );
	mMeshRayCollider.SetClosestHit( true );
	mMeshRayCollider.SetCulling( false );
	mMeshRayCollider.SetMaxDist();
}

bool CMeshRayCollider::perform( CCollisionMesh& mesh, CCollisionRay& ray, CCollisionResult& result )
{
	mFaces.Reset();
	mMeshRayCollider.SetDestination( &mFaces );

	mMeshRayCollider.SetMaxDist( ray.getLength() );
	mMeshRayCollider.SetFirstContact( false );

	bool s = mMeshRayCollider.Collide( ray.getRay(), mesh.getOpcodeModel(), NULL, NULL );
	assert( s );

	if( mMeshRayCollider.GetContactStatus() ) {
		assert( mFaces.GetNbFaces() == 1 );

		const Opcode::CollisionFace& f = *mFaces.GetFaces();
		int tri = f.mFaceID;
		float t = f.mDistance;
		result.mPosition = ray.getPosition() + ray.getDirection() * t;

		SVector3 v1, v2, v3, vn;
		mesh.getTriangle( f.mFaceID, v1, v2, v3, vn );

		result.mDirection = vn;
		result.mDistance = t;

		return true;
	} else
		return false;
}

// --------------------------------------------------------------------------

CMeshSphereCollider2::CMeshSphereCollider2()
{
	mSphereMeshCollider.SetFirstContact( false );
	mSphereMeshCollider.SetTemporalCoherence( false );

	mSphereMeshCollider2.SetFirstContact( true );
	mSphereMeshCollider2.SetTemporalCoherence( false );
}
