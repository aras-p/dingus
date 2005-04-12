// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PHYSICS_JOINT_H
#define __PHYSICS_JOINT_H

#include "../math/ODEMarshal.h"
#include "PhysContext.h"


namespace dingus {


// --------------------------------------------------------------------------

class CPhysJoint : public boost::noncopyable {
public:
	enum eType {
		JOINT_BALL = dJointTypeBall,
		JOINT_HINGE = dJointTypeHinge,
		JOINT_SLIDER = dJointTypeSlider,
		JOINT_CONTACT = dJointTypeContact,
		JOINT_UNIVERSAL = dJointTypeUniversal,
		JOINT_HINGE2 = dJointTypeHinge2,
		JOINT_FIXED = dJointTypeFixed,
		JOINT_AMOTOR = dJointTypeAMotor
	};

	enum eParam {
		PARAM_LO_STOP = dParamLoStop,
		PARAM_HI_STOP = dParamHiStop,
		PARAM_VEL = dParamVel,
		PARAM_FMAX = dParamFMax,
		PARAM_FUDGE = dParamFudgeFactor,
		PARAM_BOUNCE = dParamBounce,
		PARAM_CFM = dParamCFM,
		PARAM_STOP_ERP = dParamStopERP,
		PARAM_STOP_CFM = dParamStopCFM,
		PARAM_SUSPENSION_ERP = dParamSuspensionERP,
		PARAM_SUSPENSION_CFM = dParamSuspensionCFM,
		PARAM_LO_STOP_2 = dParamLoStop2,
		PARAM_HI_STOP_2 = dParamHiStop2,
		PARAM_VEL_2 = dParamVel2,
		PARAM_FMAX_2 = dParamFMax2,
		PARAM_FUDGE_2 = dParamFudgeFactor2,
		PARAM_BOUNCE_2 = dParamBounce2,
		PARAM_CFM_2 = dParamCFM2,
		PARAM_STOP_ERP_2 = dParamStopERP2,
		PARAM_STOP_CFM_2 = dParamStopCFM2,
		PARAM_SUSPENSION_ERP_2 = dParamSuspensionERP2,
		PARAM_SUSPENSION_CFM_2 = dParamSuspensionCFM2,
	};

public:
	CPhysJoint() : mID(0) { }
	~CPhysJoint() { if(mID) dJointDestroy(mID); } // no virtual intentionally
	
	TPhysJointID getID() const { return mID; }
	operator TPhysJointID() const { return mID; }
	
	void attach( TPhysBodyID body1, TPhysBodyID body2 ) { dJointAttach(mID, body1, body2); }
	
	//void setData (void *data) { dJointSetData (mID, data); }
	//void *getData (void *data) const { return dJointGetData (mID); }
	
	eType getType() const { return (eType)dJointGetType(mID); }
	TPhysBodyID getBody( int index ) const { return dJointGetBody(mID, index); }

protected:
	TPhysJointID mID;
};



// --------------------------------------------------------------------------

class CPhysJointBall : public CPhysJoint {
public:
	CPhysJointBall() { }
	CPhysJointBall( CPhysContext& world, TPhysJointGroupID group=0 ) { mID = dJointCreateBall(world.getID(), group); }
	void create( CPhysContext& world, TPhysJointGroupID group=0 ) {
		if(mID) dJointDestroy(mID);
		mID = dJointCreateBall(world.getID(), group);
	}
	
	void setAnchor( const SVector3& a ) { dJointSetBallAnchor(mID, a.x, a.y, a.z); }
	void getAnchor( SVector3& a ) const { dVector3 r; dJointGetBallAnchor(mID,r); odemarshal::vec3FromVector3(r,a); }
};



// --------------------------------------------------------------------------

class CPhysJointHinge : public CPhysJoint {
public:
	CPhysJointHinge() { }
	CPhysJointHinge( CPhysContext& world, TPhysJointGroupID group=0 ) { mID = dJointCreateHinge(world.getID(), group); }
	void create( CPhysContext& world, TPhysJointGroupID group=0 ) {
		if(mID) dJointDestroy(mID);
		mID = dJointCreateHinge(world.getID(), group);
	}
	
	void setAnchor( const SVector3& a ) { dJointSetHingeAnchor(mID, a.x, a.y, a.z); }
	void getAnchor( SVector3& a ) const { dVector3 r; dJointGetHingeAnchor(mID,r); odemarshal::vec3FromVector3(r,a); }
	
	void setAxis( const SVector3& a ) { dJointSetHingeAxis(mID, a.x, a.y, a.z); }
	void getAxis( SVector3& a ) const { dVector3 r; dJointGetHingeAxis(mID,r); odemarshal::vec3FromVector3(r,a); }
	
	float getAngle() const { return (float)dJointGetHingeAngle(mID); }
	float getAngleRate() const { return (float)dJointGetHingeAngleRate(mID); }
	
	//
	// params

	void	setLoStopAngle( float value ) { setParam(PARAM_LO_STOP,value); }
	void	setHiStopAngle( float value ) { setParam(PARAM_HI_STOP,value); }
	void	setStopBounce( float value ) { setParam(PARAM_BOUNCE,value); }
	void	setStopCFM( float value ) { setParam(PARAM_STOP_CFM,value); }
	void	setStopERP( float value ) { setParam(PARAM_STOP_ERP,value); }
	void	setMotorVelocity( float value ) { setParam(PARAM_VEL,value); }
	void	setMotorMaxForce( float value ) { setParam(PARAM_FMAX,value); }
	void	setFudgeFactor( float value ) { setParam(PARAM_FUDGE,value); }
	void	setCFM( float value ) { setParam(PARAM_CFM,value); }
	void	setSuspensionCFM( float value ) { setParam(PARAM_SUSPENSION_CFM,value); }
	void	setSuspensionERP( float value ) { setParam(PARAM_SUSPENSION_ERP,value); }

	float	getLoStopAngle() const { return getParam(PARAM_LO_STOP); }
	float	getHiStopAngle() const { return getParam(PARAM_HI_STOP); }
	float	getStopBounce() const { return getParam(PARAM_BOUNCE); }
	float	getStopCFM() const { return getParam(PARAM_STOP_CFM); }
	float	getStopERP() const { return getParam(PARAM_STOP_ERP); }
	float	getMotorVelocity() const { return getParam(PARAM_VEL); }
	float	getMotorMaxForce() const { return getParam(PARAM_FMAX); }
	float	getFudgeFactor() const { return getParam(PARAM_FUDGE); }
	float	getCFM() const { return getParam(PARAM_CFM); }
	float	getSuspensionCFM() const { return getParam(PARAM_SUSPENSION_CFM); }
	float	getSuspensionERP() const { return getParam(PARAM_SUSPENSION_ERP); }

private:
	void setParam( eParam param, float value ) { dJointSetHingeParam(mID, param, value); }
	float getParam( eParam param ) const { return (float)dJointGetHingeParam(mID, param); }
};


// --------------------------------------------------------------------------

class CPhysJointSlider : public CPhysJoint {
public:
	CPhysJointSlider() { }
	CPhysJointSlider( CPhysContext& world, TPhysJointGroupID group=0 ) { mID = dJointCreateSlider(world.getID(), group); }
	void create( CPhysContext& world, TPhysJointGroupID group=0 ) {
		if(mID) dJointDestroy (mID);
		mID = dJointCreateSlider(world.getID(), group);
	}
	
	void setAxis( const SVector3& a ) { dJointSetSliderAxis(mID, a.x, a.y, a.z); }
	void getAxis( SVector3& a ) const { dVector3 r; dJointGetSliderAxis(mID,r); odemarshal::vec3FromVector3(r,a); }
	
	float getPosition() const { return (float)dJointGetSliderPosition(mID); }
	float getPositionRate() const { return (float)dJointGetSliderPositionRate(mID); }
	
	//
	// params

	void	setLoStopPos( float value ) { setParam(PARAM_LO_STOP,value); }
	void	setHiStopPos( float value ) { setParam(PARAM_HI_STOP,value); }
	void	setStopBounce( float value ) { setParam(PARAM_BOUNCE,value); }
	void	setStopCFM( float value ) { setParam(PARAM_STOP_CFM,value); }
	void	setStopERP( float value ) { setParam(PARAM_STOP_ERP,value); }
	void	setMotorVelocity( float value ) { setParam(PARAM_VEL,value); }
	void	setMotorMaxForce( float value ) { setParam(PARAM_FMAX,value); }
	void	setFudgeFactor( float value ) { setParam(PARAM_FUDGE,value); }
	void	setCFM( float value ) { setParam(PARAM_CFM,value); }

	float	getLoStopPos() const { return getParam(PARAM_LO_STOP); }
	float	getHiStopPos() const { return getParam(PARAM_HI_STOP); }
	float	getStopBounce() const { return getParam(PARAM_BOUNCE); }
	float	getStopCFM() const { return getParam(PARAM_STOP_CFM); }
	float	getStopERP() const { return getParam(PARAM_STOP_ERP); }
	float	getMotorVelocity() const { return getParam(PARAM_VEL); }
	float	getMotorMaxForce() const { return getParam(PARAM_FMAX); }
	float	getFudgeFactor() const { return getParam(PARAM_FUDGE); }
	float	getCFM() const { return getParam(PARAM_CFM); }

private:
	void setParam( eParam param, float value ) { dJointSetSliderParam(mID, param, value); }
	float getParam( eParam param ) const { return (float)dJointGetSliderParam(mID, param); }
};



// --------------------------------------------------------------------------

class CPhysJointUniversal : public CPhysJoint {
public:
	CPhysJointUniversal() { }
	CPhysJointUniversal( CPhysContext& world, TPhysJointGroupID group=0 ) { mID = dJointCreateUniversal (world.getID(), group); }
	void create( CPhysContext& world, TPhysJointGroupID group=0 ) {
		if (mID) dJointDestroy (mID);
		mID = dJointCreateUniversal (world.getID(), group);
	}
	
	void setAnchor( const SVector3& a ) { dJointSetUniversalAnchor(mID, a.x, a.y, a.z); }
	void setAxis1( const SVector3& a ) { dJointSetUniversalAxis1 (mID, a.x, a.y, a.z); }
	void setAxis2( const SVector3& a ) { dJointSetUniversalAxis2 (mID, a.x, a.y, a.z); }
	
	void getAnchor( SVector3& a ) const { dVector3 r; dJointGetUniversalAnchor(mID,r); odemarshal::vec3FromVector3(r,a); }
	void getAxis1( SVector3& a ) const { dVector3 r; dJointGetUniversalAxis1(mID,r); odemarshal::vec3FromVector3(r,a); }
	void getAxis2( SVector3& a ) const { dVector3 r; dJointGetUniversalAxis2(mID,r); odemarshal::vec3FromVector3(r,a); }

	//
	// params

	void	setLoStopAngle( float value ) { setParam(PARAM_LO_STOP,value); }
	void	setHiStopAngle( float value ) { setParam(PARAM_HI_STOP,value); }
	void	setStopBounce( float value ) { setParam(PARAM_BOUNCE,value); }
	void	setStopCFM( float value ) { setParam(PARAM_STOP_CFM,value); }
	void	setStopERP( float value ) { setParam(PARAM_STOP_ERP,value); }
	void	setMotorVelocity( float value ) { setParam(PARAM_VEL,value); }
	void	setMotorMaxForce( float value ) { setParam(PARAM_FMAX,value); }
	void	setFudgeFactor( float value ) { setParam(PARAM_FUDGE,value); }
	void	setCFM( float value ) { setParam(PARAM_CFM,value); }

	void	setLoStop2Angle( float value ) { setParam(PARAM_LO_STOP_2,value); }
	void	setHiStop2Angle( float value ) { setParam(PARAM_HI_STOP_2,value); }
	void	setStop2Bounce( float value ) { setParam(PARAM_BOUNCE_2,value); }
	void	setStop2CFM( float value ) { setParam(PARAM_STOP_CFM_2,value); }
	void	setStop2ERP( float value ) { setParam(PARAM_STOP_ERP_2,value); }
	void	setMotor2Velocity( float value ) { setParam(PARAM_VEL_2,value); }
	void	setMotor2MaxForce( float value ) { setParam(PARAM_FMAX_2,value); }
	void	setFudgeFactor2( float value ) { setParam(PARAM_FUDGE_2,value); }
	void	setCFM2( float value ) { setParam(PARAM_CFM_2,value); }

	float	getLoStopAngle() const { return getParam(PARAM_LO_STOP); }
	float	getHiStopAngle() const { return getParam(PARAM_HI_STOP); }
	float	getStopBounce() const { return getParam(PARAM_BOUNCE); }
	float	getStopCFM() const { return getParam(PARAM_STOP_CFM); }
	float	getStopERP() const { return getParam(PARAM_STOP_ERP); }
	float	getMotorVelocity() const { return getParam(PARAM_VEL); }
	float	getMotorMaxForce() const { return getParam(PARAM_FMAX); }
	float	getFudgeFactor() const { return getParam(PARAM_FUDGE); }
	float	getCFM() const { return getParam(PARAM_CFM); }

	float	getLoStop2Angle() const { return getParam(PARAM_LO_STOP_2); }
	float	getHiStop2Angle() const { return getParam(PARAM_HI_STOP_2); }
	float	getStop2Bounce() const { return getParam(PARAM_BOUNCE_2); }
	float	getStop2CFM() const { return getParam(PARAM_STOP_CFM_2); }
	float	getStop2ERP() const { return getParam(PARAM_STOP_ERP_2); }
	float	getMotor2Velocity() const { return getParam(PARAM_VEL_2); }
	float	getMotor2MaxForce() const { return getParam(PARAM_FMAX_2); }
	float	getFudge2Factor() const { return getParam(PARAM_FUDGE_2); }
	float	getCFM2() const { return getParam(PARAM_CFM_2); }

private:
	void setParam( eParam param, float value ) { dJointSetUniversalParam(mID, param, value); }
	float getParam( eParam param ) const { return (float)dJointGetUniversalParam(mID, param); }
};



// --------------------------------------------------------------------------

class CPhysJointHinge2 : public CPhysJoint {
public:
	CPhysJointHinge2() { }
	CPhysJointHinge2( CPhysContext& world, TPhysJointGroupID group=0 ) { mID = dJointCreateHinge2 (world.getID(), group); }
	void create( CPhysContext& world, TPhysJointGroupID group=0 ) {
		if (mID) dJointDestroy (mID);
		mID = dJointCreateHinge2 (world.getID(), group);
	}
	
	void setAnchor( const SVector3& a ) { dJointSetHinge2Anchor(mID, a.x, a.y, a.z); }
	void setAxis1( const SVector3& a ) { dJointSetHinge2Axis1 (mID, a.x, a.y, a.z); }
	void setAxis2( const SVector3& a ) { dJointSetHinge2Axis2 (mID, a.x, a.y, a.z); }
	
	void getAnchor( SVector3& a ) const { dVector3 r; dJointGetHinge2Anchor(mID,r); odemarshal::vec3FromVector3(r,a); }
	void getAxis1( SVector3& a ) const { dVector3 r; dJointGetHinge2Axis1(mID,r); odemarshal::vec3FromVector3(r,a); }
	void getAxis2( SVector3& a ) const { dVector3 r; dJointGetHinge2Axis2(mID,r); odemarshal::vec3FromVector3(r,a); }

	float getAngle1() const { return (float)dJointGetHinge2Angle1(mID); }
	float getAngle1Rate() const { return (float)dJointGetHinge2Angle1Rate(mID); }
	float getAngle2Rate() const { return (float)dJointGetHinge2Angle2Rate(mID); }
	
	//
	// params

	void	setLoStopAngle( float value ) { setParam(PARAM_LO_STOP,value); }
	void	setHiStopAngle( float value ) { setParam(PARAM_HI_STOP,value); }
	void	setStopBounce( float value ) { setParam(PARAM_BOUNCE,value); }
	void	setStopCFM( float value ) { setParam(PARAM_STOP_CFM,value); }
	void	setStopERP( float value ) { setParam(PARAM_STOP_ERP,value); }
	void	setMotorVelocity( float value ) { setParam(PARAM_VEL,value); }
	void	setMotorMaxForce( float value ) { setParam(PARAM_FMAX,value); }
	void	setFudgeFactor( float value ) { setParam(PARAM_FUDGE,value); }
	void	setCFM( float value ) { setParam(PARAM_CFM,value); }
	void	setSuspensionCFM( float value ) { setParam(PARAM_SUSPENSION_CFM,value); }
	void	setSuspensionERP( float value ) { setParam(PARAM_SUSPENSION_ERP,value); }

	void	setMotor2Velocity( float value ) { setParam(PARAM_VEL_2,value); }
	void	setMotor2MaxForce( float value ) { setParam(PARAM_FMAX_2,value); }
	void	setCFM2( float value ) { setParam(PARAM_CFM_2,value); }

	float	getLoStopAngle() const { return getParam(PARAM_LO_STOP); }
	float	getHiStopAngle() const { return getParam(PARAM_HI_STOP); }
	float	getStopBounce() const { return getParam(PARAM_BOUNCE); }
	float	getStopCFM() const { return getParam(PARAM_STOP_CFM); }
	float	getStopERP() const { return getParam(PARAM_STOP_ERP); }
	float	getMotorVelocity() const { return getParam(PARAM_VEL); }
	float	getMotorMaxForce() const { return getParam(PARAM_FMAX); }
	float	getFudgeFactor() const { return getParam(PARAM_FUDGE); }
	float	getCFM() const { return getParam(PARAM_CFM); }
	float	getSuspensionCFM() const { return getParam(PARAM_SUSPENSION_CFM); }
	float	getSuspensionERP() const { return getParam(PARAM_SUSPENSION_ERP); }

	float	getMotor2Velocity() const { return getParam(PARAM_VEL_2); }
	float	getMotor2MaxForce() const { return getParam(PARAM_FMAX_2); }
	float	getCFM2() const { return getParam(PARAM_CFM_2); }

private:
	void setParam( eParam param, float value ) { dJointSetHinge2Param(mID, param, value); }
	float getParam( eParam param ) const { return (float)dJointGetHinge2Param(mID, param); }
};


/*
class dFixedJoint : public CPhysJoint {
public:
	dFixedJoint() { }
	dFixedJoint (CPhysContext& world, TPhysJointGroupID group=0)
	{ mID = dJointCreateFixed (world.getID(), group); }
	
	void create (CPhysContext& world, TPhysJointGroupID group=0) {
		if (mID) dJointDestroy (mID);
		mID = dJointCreateFixed (world.getID(), group);
	}
	
	void set()
	{ dJointSetFixed (mID); }
};
*/

/*
class CPhysJointContact : public CPhysJoint {
public:
	CPhysJointContact() { }
	CPhysJointContact( CPhysContext& world, TPhysJointGroupID group, dContact *contact ) { mID = dJointCreateContact(world.getID(), group, contact); }
	
	void create( CPhysContext& world, TPhysJointGroupID group, dContact *contact ) {
		if(mID) dJointDestroy (mID);
		mID = dJointCreateContact(world.getID(), group, contact);
	}
};
*/



}; // namespace

#endif
