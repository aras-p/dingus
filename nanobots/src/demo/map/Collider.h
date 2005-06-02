#ifndef __COLLIDER_H
#define __COLLIDER_H

#include <opcode/Opcode.h>

// --------------------------------------------------------------------------

inline SVector3 convert( const Point& vertex ) 
{
	return SVector3( vertex.x, vertex.y, vertex.z );
}

inline Point convert( const SVector3& vertex )
{
	return Point( vertex.x, vertex.y, vertex.z );
}


// --------------------------------------------------------------------------


class CCollisionMesh {
public:
	CCollisionMesh( int nverts, int ntris, const SVector3* verts, int vertStride, const unsigned short* indices, bool indicesAreActuallyInts = false );
	~CCollisionMesh();

	int getVertexCount() const { return mVertexCount; }
	int getTriangleCount() const { return mTriangleCount; }
	const SVector3& getVertex( int index ) const { return mVertices[index]; }
	int getIndex( int index ) const { return mIndices[index]; }
	const int* getTriangle( int index ) const { return &mIndices[index*3]; }

	Opcode::Model& getOpcodeModel() { return mModel; }
	IndexedTriangle* getOpcodeTriangles() const { return (IndexedTriangle*)mIndices; }
	IceMaths::Point* getOpcodeVertices() const { return (IceMaths::Point*)mVertices; }

	/**
	 * v1, v2, v3 - vertices in world space of triangle with index triangleIndex
	 * normal - normalized normal in world space of triangle with index triangleIndex
	 */
	void getTriangle( int triangleIndex, SVector3& v1, SVector3& v2, SVector3& v3, SVector3& normal ) const;

private:
	Opcode::Model mModel;
	int*		mIndices;
	SVector3*	mVertices;
	int			mVertexCount;
	int			mTriangleCount;
};


// --------------------------------------------------------------------------

class CCollisionRay {
public:
	CCollisionRay( const SVector3& position = SVector3(0,0,0), const SVector3& direction = SVector3(1,0,0) );
	
	const Ray& getRay() const { return mRay; }

	SVector3 getPosition() const { return convert( mRay.mOrig ); }
	SVector3 getDirection() const { return convert( mRay.mDir ); }
	float	 getLength() const { return mLength; }
	void setPosition( const SVector3& position ) { mRay.mOrig = convert( position ); }
	void setDirection( const SVector3& direction ); 
	void setLength( float l = MAX_FLOAT ) { mLength = l; }
	void setRay( const SVector3& posA, const SVector3& posB ) {
		SVector3 a2b = posB - posA;
		setPosition( posA );
		setDirection( a2b );
		setLength( a2b.length() );
	}
private:
	Ray		mRay;
	float	mLength;
};


// --------------------------------------------------------------------------


class CCollisionSphere  {
public:
	CCollisionSphere( float radius = 1.0f );

	const Sphere& getSphere() const { return mSphere; }

	SVector3 getPosition() const { return convert(mSphere.Center()); }
	void setPosition( const SVector3& position ) { mSphere.mCenter = convert( position ); }

	float getRadius() const { return mSphere.GetRadius(); }
	void setRadius( float r ) { mSphere.SetRadius( r ); }

private:
	Sphere mSphere;
};


// --------------------------------------------------------------------------

class CCollisionResult {
public:

	CCollisionResult() : mPosition(0,0,0), mDirection(0,1,0), mDistance( 0 ) { }
	
	void swap() { mDirection = -mDirection; }

public:
	/// position of collision
	SVector3	mPosition;
	//// collision direction pointing from first object to second
	SVector3 mDirection;
	/// depth of intesection or distance along ray
	float		mDistance;
};


// --------------------------------------------------------------------------

class CMeshRayCollider {
public:
	CMeshRayCollider();
	bool perform( CCollisionMesh& mesh, CCollisionRay& ray, CCollisionResult& result );
private:
	Opcode::RayCollider		mMeshRayCollider;
	Opcode::CollisionFaces	mFaces;
};



class CMeshSphereCollider2 {
public:
	CMeshSphereCollider2();

	bool perform( CCollisionMesh& mesh, CCollisionSphere& sphere, CCollisionResult& result );
	void perform( CCollisionMesh& mesh, CCollisionSphere& sphere, std::vector<int>& tris );

private:
	typedef std::vector<SVector3> TVec3Vector;
private:
	Opcode::SphereCollider mSphereMeshCollider; // for full collision
	Opcode::SphereCollider mSphereMeshCollider2; // for collision status only
	TVec3Vector	mNormals;
	CCollisionResult mResult;
};



#endif
