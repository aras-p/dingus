// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#ifndef __PHYSICS_BODY_H
#define __PHYSICS_BODY_H

#include "../math/ODEMarshal.h"
#include "PhysContext.h"


namespace dingus {


struct SPhysicsMass {
public:
	SPhysicsMass() : mass(1.0f) { inertiaTensor.identify(); }
public:
	SMatrix4x4	inertiaTensor;
	float		mass;
};



// --------------------------------------------------------------------------

class CPhysBody : public boost::noncopyable {
public:
	CPhysBody() : mID(0) { }
	CPhysBody( CPhysContext& world ) { mID = dBodyCreate(world.getID()); }
	~CPhysBody() { if(mID) dBodyDestroy(mID); } // no virtual intentionally
	void create( CPhysContext& world ) {
		if(mID) dBodyDestroy (mID);
		mID = dBodyCreate(world.getID());
	}
	
	TPhysBodyID getID() const { return mID; }
	operator TPhysBodyID() const { return mID; }
	
	void setData( void *data ) { dBodySetData(mID, data); }
	void *getData() const { return dBodyGetData(mID); }
	
	/**
	 *  Places body at specified position/orientation, resets it's velocities
	 *  and forces/torques.
	 */
	void place( const SMatrix4x4& worldmat ) {
		setPosition( worldmat.getOrigin() );
		setRotation( worldmat );
		dBodySetLinearVel( mID, 0, 0, 0 );
		dBodySetAngularVel( mID, 0, 0, 0 );
		dBodySetForce( mID, 0, 0, 0 );
		dBodySetTorque( mID, 0, 0, 0 );
	}
	
	void setPosition( const SVector3& pos ) { dBodySetPosition(mID,pos.x,pos.y,pos.z); }
	void setRotation( const SMatrix4x4& rot );
	//void setRotation( const SQuaternion& q ) { dBodySetQuaternion(mID,q); }
	void setLinearVel( const SVector3& v ) { dBodySetLinearVel(mID,v.x,v.y,v.z); }
	void setAngularVel( const SVector3& v ) { dBodySetAngularVel(mID,v.x,v.y,v.z); }
	
	SVector3 getPosition() const { const dReal* v = dBodyGetPosition(mID); return odemarshal::vec3FromPtr( v ); }
	void getPosition( SVector3& dest ) const { const dReal* v = dBodyGetPosition(mID); odemarshal::vec3FromPtr( v, dest ); }
	//SQuaternion getRotation() const { return SQuaternion( dBodyGetQuaternion(mID) ); }
	void getRotation( SMatrix4x4& dest ) const;
	SVector3 getLinearVel() const { const dReal* v = dBodyGetLinearVel(mID); return odemarshal::vec3FromPtr( v ); }
	SVector3 getAngularVel() const { const dReal* v = dBodyGetAngularVel(mID); return odemarshal::vec3FromPtr( v ); }

	void	setMass( const SPhysicsMass& mass );
	
	void	addForce( const SVector3& f ) { dBodyAddForce(mID, f.x, f.y, f.z); }
	void	addTorque( const SVector3& t ) { dBodyAddTorque(mID, t.x, t.y, t.z); }
	void	addLocalForce( const SVector3& f ) { dBodyAddRelForce(mID, f.x, f.y, f.z); }
	void	addLocalTorque( const SVector3& t ) { dBodyAddRelTorque(mID, t.x, t.y, t.z); }
	void	addForceAtPos( const SVector3& f, const SVector3& pos ) { dBodyAddForceAtPos(mID, f.x, f.y, f.z, pos.x, pos.y, pos.z); }
	void	addForceAtLocalPos( const SVector3& f, const SVector3& pos ) { dBodyAddForceAtRelPos(mID, f.x, f.y, f.z, pos.x, pos.y, pos.z); }
	void	addLocalForceAtPos( const SVector3& f, const SVector3& pos ) { dBodyAddRelForceAtPos(mID, f.x, f.y, f.z, pos.x, pos.y, pos.z); }
	void	addLocalForceAtLocalPos( const SVector3& f, const SVector3& pos ) { dBodyAddRelForceAtRelPos(mID, f.x, f.y, f.z, pos.x, pos.y, pos.z); }
	
	SVector3 getForce() const { const dReal* v = dBodyGetForce(mID); return odemarshal::vec3FromPtr( v ); }
	SVector3 getTorque() const { const dReal* v = dBodyGetTorque(mID); return odemarshal::vec3FromPtr( v ); }
	void	setForce( const SVector3& f ) { dBodySetForce(mID,f.x,f.y,f.z); }
	void	setTorque( const SVector3& t ) { dBodySetTorque(mID,t.x,t.y,t.z); }
	
	void	enable() { dBodyEnable (mID); }
	void	disable() { dBodyDisable (mID); }
	bool	isEnabled() const { return dBodyIsEnabled(mID)?true:false; }
	void	setAutoDisable(
		bool autodisable, int steps, float time,
		float linearThr, float angularThr );
	void	setDefaultAutoDisable();
	
	///** Given local point, gets it's world position. */
	//void	getLocalPtPos( const SVector3& p, SVector3& res ) const { dBodyGetRelPointPos(mID, p.x,p.y,p.z, res); }
	///** Given local point, gets it's world velocity. */
	//void	getLocalPtVel( const SVector3& p, SVector3& res ) const { dBodyGetRelPointVel(mID, p.x,p.y,p.z, res); }
	///** Given world point, gets it's world velocity. */
	//void	getPtVel( const SVector3& p, SVector3& res ) const { dBodyGetPointVel(mID, p.x,p.y,p.z, res); }
	///** Given world point, gets it's local position. */
	//void	getPtLocalPos( const SVector3& p, SVector3& res ) const { dBodyGetPosRelPoint(mID, p.x,p.y,p.z, res); }
	///** Rotates local vector into world. */
	//void	vectorToWorld( const SVector3& p, SVector3& res ) const { dBodyVectorToWorld(mID, p.x,p.y,p.z, res); }
	///** Rotates world vector into local. */
	//void	vectorFromWorld( const SVector3& p, SVector3& res ) const { dBodyVectorFromWorld(mID, p.x,p.y,p.z, res); }
	
	
	void setFiniteRotationMode( bool finite ) { dBodySetFiniteRotationMode(mID, finite?1:0); }
	bool isFiniteRotationMode() const { return dBodyGetFiniteRotationMode(mID)?true:false; }
	void setFiniteRotationAxis( const SVector3& a ) { dBodySetFiniteRotationAxis (mID, a.x, a.y, a.z); }
	void getFiniteRotationAxis( SVector3& res ) const { dVector3 r; dBodyGetFiniteRotationAxis(mID, r); odemarshal::vec3FromPtr( r, res ); }
	
	int getJointCount() const { return dBodyGetNumJoints(mID); }
	//TJointID getJoint(int index) const { return dBodyGetJoint(mID, index); }
	
	void setGravitable( bool g ) { dBodySetGravityMode(mID,g?1:0); }
	bool isGravitable() const { return dBodyGetGravityMode(mID)?true:false; }
	
	bool isConnectedTo( TPhysBodyID body ) const { return dAreConnected(mID, body)?true:false; }

private:
	TPhysBodyID mID;
};


inline void CPhysBody::setAutoDisable(
		bool autodisable, int steps, float time,
		float linearThr, float angularThr )
{
	dBodySetAutoDisableFlag( mID, autodisable );
	dBodySetAutoDisableSteps( mID, steps );
	dBodySetAutoDisableTime( mID, time );
	dBodySetAutoDisableLinearThreshold( mID, linearThr );
	dBodySetAutoDisableAngularThreshold( mID, angularThr );
}

inline void CPhysBody::setDefaultAutoDisable()
{
	assert( mID );
	dBodySetAutoDisableDefaults( mID );
}

}; // namespace

#endif
