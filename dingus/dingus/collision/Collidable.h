// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __COLLIDABLE_H
#define __COLLIDABLE_H

#include "../math/ODEMarshal.h"
#include "../math/AABox.h"
#include "../math/Plane.h"
#include "CollisionMesh.h"
#include "CollisionListener.h"
#include "../utils/Notifier.h"

namespace dingus {



// --------------------------------------------------------------------------

class CCollidable : public CNotifier<ICollisionListener>, public boost::noncopyable {
public:
	enum eClass {
		COL_SPHERE = dSphereClass,
		COL_BOX = dBoxClass,
		COL_CAPSULE = dCCylinderClass,
		COL_CYLINDER = dCylinderClass,
		COL_PLANE = dPlaneClass,
		COL_TRANSFORM = dGeomTransformClass,
		COL_RAY = dRayClass,
		COL_MESH = dTriMeshClass,
		COL_SIMPLE_CONT = dSimpleSpaceClass,
		COL_HASH_CONT = dHashSpaceClass,
		COL_QUAD_CONT = dQuadTreeSpaceClass,
		//COL_SAP_CONT = dSweepAndPruneSpaceClass
	};

public:
	~CCollidable() { if( mID ) dGeomDestroy(mID); } /* no virtual */
	//void	destroy() { if(mID) dGeomDestroy(mID); mID = 0; }
	
	TCollidableID	getID() const { return mID; }
	operator TCollidableID() const { return mID; }
	bool operator<( const CCollidable& c ) const { return (unsigned int)mID < (unsigned int)c.mID; }
	
	void	setUserData( void *data ) { mUserData = data; }
	void*	getUserData() const { return mUserData; }

	void	setPhysBody( TPhysBodyID p ) { dGeomSetBody(mID,p); }
	TPhysBodyID getPhysBody() { return dGeomGetBody(mID); }

	eClass	getClass() const { return (eClass)dGeomGetClass(mID); }
	
	void	setPosition( const SVector3& pos ) { dGeomSetPosition(mID,pos.x,pos.y,pos.z); }
	void	getPosition( SVector3& pos ) const { const dReal* v = dGeomGetPosition(mID); odemarshal::vec3FromPtr(v,pos); }
	void	setRotation( const SMatrix4x4& rot );
	void	getRotation( SMatrix4x4& rot ) const;

	void	getAABB( CAABox& ab ) const {
		double aabb[6];
		dGeomGetAABB( mID, aabb );
		ab.getMin().set( (float)aabb[0], (float)aabb[2], (float)aabb[4] );
		ab.getMax().set( (float)aabb[1], (float)aabb[3], (float)aabb[5] );
	}
	
	bool	isSpace() const { return dGeomIsSpace(mID) ? true : false; }
	
	void	setCategoryBits( unsigned long bits ) { dGeomSetCategoryBits(mID, bits); }
	void	setCollideBits( unsigned long bits ) { dGeomSetCollideBits(mID, bits); }
	unsigned long getCategoryBits() const { return dGeomGetCategoryBits(mID); }
	unsigned long getCollideBits() const { return dGeomGetCollideBits(mID); }
	
	void	enable() { dGeomEnable(mID); }
	void	disable() { dGeomDisable(mID); }
	int		isEnabled() const { return dGeomIsEnabled(mID); }
	
	void notifyListeners( CCollidable& him ) {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it )
			(*it)->onCollide( *this, him );
	}

	/** @return number of contacts. */
	int collideWith( CCollidable& other, int maxContacts, dContactGeom* contacts ) {
		return dCollide( mID, other, maxContacts, contacts, sizeof(dContactGeom) );
	}

protected:
	CCollidable( TCollidableID id ) : mID(id), mUserData(0) { dGeomSetData(mID,this); }
protected:
	TCollidableID	mID;
	// We store pointer to (this) in ODE's data, and user data here
	void*			mUserData;
};



// --------------------------------------------------------------------------

class CCollidableContainer : public CCollidable {
public:
	TCollidableContainerID getID() const { return (TCollidableContainerID)mID; }
	operator TCollidableContainerID() const { return (TCollidableContainerID) mID; }
	
	void setCleanup( bool cleanup ) { dSpaceSetCleanup( getID(), cleanup?1:0 ); }
	bool isCleanup() const { return dSpaceGetCleanup(getID()) ? true:false; }
	
	void add( CCollidable& x ) { dSpaceAdd(getID(),x); }
	void remove( CCollidable& x ) { dSpaceRemove(getID(),x); }
	bool contains( CCollidable const& x ) const { return dSpaceQuery(getID(),x) ? true:false; }
	
	int size() const { return dSpaceGetNumGeoms(getID()); }
	TCollidableID getCollidable( int i ) { return dSpaceGetGeom(getID(),i); }
	
	void collideSelf( void *data, dNearCallback *callback ) { dSpaceCollide(getID(),data,callback); }

protected:
	CCollidableContainer( TCollidableContainerID id ) : CCollidable( (TCollidableID)id ) { }
};



// --------------------------------------------------------------------------

class CSimpleCollidableContainer : public CCollidableContainer {
public:
	CSimpleCollidableContainer( TCollidableContainerID parent )
		: CCollidableContainer( dSimpleSpaceCreate(parent) ) { }
};



// --------------------------------------------------------------------------

class CHashCollidableContainer : public CCollidableContainer {
public:
	CHashCollidableContainer( TCollidableContainerID parent )
		: CCollidableContainer( dHashSpaceCreate(parent) ) { }
	void setLevels( int minlevel, int maxlevel ) { dHashSpaceSetLevels(getID(),minlevel,maxlevel); }
};

// --------------------------------------------------------------------------

class CQuadtreeCollidableContainer : public CCollidableContainer {
public:
	CQuadtreeCollidableContainer( TCollidableContainerID parent, const CAABox& aabb, int levels = 5 )
		: CCollidableContainer(
			dQuadTreeSpaceCreate(
				parent,
				odemarshal::vec3ToVector3( aabb.getCenter() ).v,
				odemarshal::vec3ToVector3( (aabb.getMax()-aabb.getMin())*0.5f ).v,
				levels ) ) { }
};



// --------------------------------------------------------------------------

/**
 *  Collidable sphere with radius.
 *  Positioned at matrix origin.
 */
class CCollidableSphere : public CCollidable {
public:
	CCollidableSphere( TCollidableContainerID parent, float radius )
		: CCollidable( dCreateSphere(parent,radius) ) { }
	
	void setRadius( float radius ) { dGeomSphereSetRadius(mID, radius); }
	float getRadius() const { return (float)dGeomSphereGetRadius(mID); }
};



// --------------------------------------------------------------------------

/**
 *  Collidable box.
 *  Positioned at matrix origin, axis aligned.
 */
class CCollidableBox : public CCollidable {
public:
	CCollidableBox( TCollidableContainerID parent, const SVector3& len )
		: CCollidable( dCreateBox( parent, len.x, len.y, len.z ) ) { }

	void setLen( const SVector3& len ) { dGeomBoxSetLengths(mID,len.x,len.y,len.z); }
	void getLen( SVector3& len ) const { dVector3 l; dGeomBoxGetLengths(mID,l); odemarshal::vec3FromVector3(l,len); }
	SVector3 getLen() const { dVector3 l; dGeomBoxGetLengths(mID,l); return odemarshal::vec3FromVector3(l); }
};


// --------------------------------------------------------------------------

/**
 *  Collidable capsule.
 *  Centered at matrix origin, aligned with matrix Z axis.
 */
class CCollidableCapsule : public CCollidable {
public:
	CCollidableCapsule( TCollidableContainerID parent, float radius, float length )
		: CCollidable( dCreateCCylinder( parent, radius, length ) ) { }

	void setParams( float radius, float length ) { dGeomCCylinderSetParams( mID, radius, length ); }
	void getParams( float& radius, float& length ) const { dReal r, l; dGeomCCylinderGetParams( mID, &r, &l ); radius=float(r); length=float(l); }
};



// --------------------------------------------------------------------------

/**
 *  Collidable plane.
 *  Can't be positioned, always expressed by plane equation.
 */
class CCollidablePlane : public CCollidable {
public:
	CCollidablePlane( TCollidableContainerID parent, const SPlane& p )
		: CCollidable( dCreatePlane(parent, p.a, p.b, p.c, p.d) ) { }

	void setPlane( const SPlane& p ) { dGeomPlaneSetParams(mID, p.a, p.b, p.c, p.d); }
	void getParams( SPlane& res ) const { dVector3 r; dGeomPlaneGetParams(mID,r); res.a=(float)r[0];res.b=(float)r[1];res.c=(float)r[2];res.d=(float)r[3]; }
};


// --------------------------------------------------------------------------

/**
 *  Collidable ray.
 *  Starts at matrix origin, extends in direction of matrix Z axis.
 */
class CCollidableRay : public CCollidable {
public:
	CCollidableRay( TCollidableContainerID parent, float length )
		: CCollidable( dCreateRay(parent,length) ) { }

	void	setLength( float length ) { dGeomRaySetLength(mID,length); }
	float	getLength() const { return (float)dGeomRayGetLength(mID); }
	
	void	setParams( const SVector3& pos, const SVector3& dir ) { dGeomRaySet(mID,pos.x,pos.y,pos.z,dir.x,dir.y,dir.z); }
	void	getParams( SVector3& pos, SVector3& dir ) const;
};



// --------------------------------------------------------------------------

class CCollidableMesh : public CCollidable {
public:
	CCollidableMesh( TCollidableContainerID parent, const CCollisionMesh& mesh )
		: CCollidable( dCreateTriMesh( parent, mesh.getTriMeshData(), NULL, NULL, NULL ) ) { }
};



// --------------------------------------------------------------------------

/**
 *  Collidable transform.
 *  Encapsulates and transforms as a child another collidable.
 *  The other collidable must NOT be inserted into any space or associated
 *  with a body.
 */
class CCollidableTransform : public CCollidable {
public:
	CCollidableTransform( TCollidableContainerID parent )
		: CCollidable( dCreateGeomTransform(parent) )
	{
		dGeomTransformSetInfo( mID, 1 );
	}
	
	void setCollidable( TCollidableID coll ) { dGeomTransformSetGeom(mID, coll); }
	CCollidable* getCollidable() const { return (CCollidable*)dGeomGetData( dGeomTransformGetGeom(mID) ); }
};
	
}; // namespace


#endif
