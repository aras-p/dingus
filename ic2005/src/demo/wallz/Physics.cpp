#include "stdafx.h"
#include "Physics.h"
#include <dingus/math/ODEMarshal.h>
#include <dingus/math/Plane.h>
#include <dingus/utils/CpuTimer.h>
#include <dingus/utils/Random.h>
#include <dingus/math/MathUtils.h>
#include "../DemoResources.h"

using namespace physics;


// --------------------------------------------------------------------------
//  ODE
// --------------------------------------------------------------------------

#ifdef PHYSICS_ODE


namespace {

	SStats			stats;

	
	dWorldID		world;
	dSpaceID		space;
	dJointGroupID	contacts;

	float			updateDT;

	enum eColClass {
		COLCLASS_WALL = (1<<0),
		COLCLASS_PIECESTART = 1,
		COLCLASS_PIECECOUNT = 8,
	};

	std::vector<dGeomID>	planes;


}; // namespace



CPhysObject::CPhysObject( const SMatrix4x4& matrix, const SVector3& size )
{
	// construct body and geom
	mObject = dBodyCreate( world );
	mCollidable = dCreateBox( space, size.x, size.y, size.z );
	static int counter = 0;
	counter = (counter+1) % COLCLASS_PIECECOUNT;
	dGeomSetCategoryBits( mCollidable, (1<<(COLCLASS_PIECESTART+counter)) );
	dGeomSetCollideBits( mCollidable, (1<<(COLCLASS_PIECESTART+counter)) | COLCLASS_WALL );
	dMatrix3 rot;
	odemarshal::matrixToMatrix3( matrix, rot );
	dBodySetPosition( mObject, matrix.getOrigin().x, matrix.getOrigin().y, matrix.getOrigin().z );
	dBodySetRotation( mObject, rot );

	dGeomSetBody( mCollidable, mObject );

	dMass mass;
	dMassSetBoxTotal( &mass, 1.0f, size.x, size.y, size.z );
	dBodySetMass( mObject, &mass );

	//SVector3 force( gRandom.getFloat(-50,50), gRandom.getFloat(-50,50), gRandom.getFloat(30,40) );
	// force to pull all inside the room
	SVector3 towards = ROOM_MID - matrix.getOrigin();
	towards.normalize();
	
	SVector3 force = towards * 70;
	force.x += gRandom.getFloat(-50,50);
	force.y += gRandom.getFloat(-50,50);
	force.z += gRandom.getFloat(-50,50);

	SVector3 fpos( gRandom.getFloat(-size.x,size.x), gRandom.getFloat(-size.y,size.y), gRandom.getFloat(-size.z,size.z) );
	dBodyAddForceAtRelPos( mObject, force.x, force.y, force.z, fpos.x, fpos.y, fpos.z );
}

CPhysObject::~CPhysObject()
{
	removeFromPhysics();
}

void CPhysObject::update( SMatrix4x4& matrix )
{
	if( isRemoved() )
		return;

	// decrease mObject's linear velocity
	const float LVEL_FACTOR = 0.994f;
	const dReal* lvel = dBodyGetLinearVel( mObject );
	dBodySetLinearVel( mObject, lvel[0]*LVEL_FACTOR, lvel[1]*LVEL_FACTOR, lvel[2]*LVEL_FACTOR );

	// decrease mObject's angular velocity
	const float AVEL_FACTOR = 0.94f;
	const dReal* avel = dBodyGetAngularVel( mObject );
	D3DXVECTOR3 newAVel( avel[0]*AVEL_FACTOR, avel[1]*AVEL_FACTOR, avel[2]*AVEL_FACTOR );

	// if our velocities got small enough and we're still stuck in the air,
	// apply random force
	if( matrix.getOrigin().y > 0.4f ) {
		const float lvelL = lvel[0]*lvel[0]+lvel[1]*lvel[1]+lvel[2]*lvel[2];
		//const float avelL = avel[0]*avel[0]+avel[1]*avel[1]+avel[2]*avel[2];
		if( lvelL < 0.3f*0.3f /*|| avelL < 0.5f*0.5f*/ ) {
			SVector3 towards = ROOM_MID - matrix.getOrigin();
			towards.normalize();
			
			SVector3 force = towards * 50;
			force.x += gRandom.getFloat(-20,20);
			force.y += gRandom.getFloat(-20,20);
			force.z += gRandom.getFloat(-20,20);

			dBodyAddForce( mObject, force.x, force.y, force.z );
		}
	}

	// Also clamp angular velocity. Some light and elongated
	// objects tend to spin very quickly at deep collisions, and for some
	// reason the spinning accelerates to infinities (floating point?).
	const float CLAMP_AVEL = 15.0f;
	newAVel.x = clamp( newAVel.x, -CLAMP_AVEL, CLAMP_AVEL );
	newAVel.y = clamp( newAVel.y, -CLAMP_AVEL, CLAMP_AVEL );
	newAVel.z = clamp( newAVel.z, -CLAMP_AVEL, CLAMP_AVEL );
	dBodySetAngularVel( mObject, newAVel.x, newAVel.y, newAVel.z );

	// get matrix
	const float* pos = dBodyGetPosition( mObject );
	const float* rot = dBodyGetRotation( mObject );
	odemarshal::matrixFromMatrix3( rot, matrix );
	odemarshal::vec3FromVector3( pos, matrix.getOrigin() );
}

void CPhysObject::removeFromPhysics()
{
	if( !isRemoved() ) {
		dBodyDestroy( mObject );
		mObject = NULL;
		dGeomDestroy( mCollidable );
		mCollidable = NULL;
	}
}

bool CPhysObject::isIdle() const
{
	if( isRemoved() )
		return true;
	return dBodyIsEnabled(mObject) ? false : true;
}


void physics::initialize( float updDT, float grav, const SVector3& boundMin, const SVector3& boundMax )
{
	updateDT = updDT;

	world = dWorldCreate();
	
	//space = dHashSpaceCreate( NULL );
	//dHashSpaceSetLevels( space, -6, 0 );

	//SVector3 boundCenter = (boundMin+boundMax)/2;
	//SVector3 boundExt = (boundMax-boundMin);
	//space = dQuadTreeSpaceCreate( NULL, boundCenter, boundExt, 6 );

	space = dSweepAndPruneSpaceCreate( NULL, dSAP_AXES_XZY );

	contacts = dJointGroupCreate( 0 );

	dWorldSetGravity( world, 0, grav, 0 );
	dWorldSetERP( world, 0.4f );
	dWorldSetCFM( world, 1.0e-4f );

	dWorldSetQuickStepNumIterations( world, 4 );

	dWorldSetContactMaxCorrectingVel( world, 1.0e4f );
	dWorldSetContactSurfaceLayer( world, 0.001f );

	dWorldSetAutoDisableFlag( world, 1 );
	dWorldSetAutoDisableSteps( world, 20 );
	dWorldSetAutoDisableTime( world, 0.3f );
	dWorldSetAutoDisableLinearThreshold( world, 0.3f );
	dWorldSetAutoDisableAngularThreshold( world, 0.4f );
}


void physics::addPlane( const SMatrix4x4& matrix )
{
	SPlane plane( matrix.getOrigin(), matrix.getAxisZ() );
	dGeomID geom = dCreatePlane( space, plane.a, plane.b, plane.c, -plane.d );
	dGeomSetCategoryBits( geom, COLCLASS_WALL );
	dGeomSetCollideBits( geom, 0 );
	planes.push_back( geom );
}


void physics::shutdown()
{
	for( int i = 0; i < planes.size(); ++i )
		dGeomDestroy( planes[i] );
	planes.clear();

	dJointGroupDestroy( contacts );
	dSpaceDestroy( space );
	dWorldDestroy( world );
}


namespace {
void	nearCallback( void *data, dGeomID o1, dGeomID o2 )
{
	int	i,n;

	dBodyID b1 = dGeomGetBody( o1 );
	dBodyID b2 = dGeomGetBody( o2 );
	bool active1 = b1 && dBodyIsEnabled( b1 );
	bool active2 = b2 && dBodyIsEnabled( b2 );

	// disabled vs disabled or disabled vs geom-only
	if( !active1 && !active2 )
		return;

	// HACK: skip disabled vs active
	if( b1 && b2 ) {
		if( !active1 || !active2 )
			return;
	}

	const int NCONTACTS = 4;
	dContact contact[NCONTACTS];
	n =	dCollide( o1, o2, NCONTACTS, &contact[0].geom, sizeof(dContact) );

	if( n < 1 )
		return;

	//const float maxDepth = 0.5f;
	const float depthMult = 0.8f;

	for( i = 0; i < n; ++i ) {
		contact[i].surface.mode	= 0; //dContactApprox1;
		contact[i].surface.mu =	10;
		//contact[i].surface.soft_erp	= 0.5;
		//contact[i].surface.soft_cfm	= 0.01;
		//contact[i].geom.depth = clamp( contact[i].geom.depth, -maxDepth, maxDepth );
		contact[i].geom.depth *= depthMult;

		dJointID c = dJointCreateContact( world, contacts, &contact[i] );
		dJointAttach( c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2) );
	}
}
}


void physics::update( int lod )
{
	{
		cputimer::ticks_type t1 = cputimer::ticks();
		dJointGroupEmpty( contacts );
		dSpaceCollide( space, NULL, nearCallback );
		cputimer::ticks_type t2 = cputimer::ticks();
		stats.msColl = double(t2-t1) * cputimer::secsPerTick() * 1000.0f;
	}
	{
		cputimer::ticks_type t1 = cputimer::ticks();
		dWorldSetQuickStepNumIterations( world, 5-lod );
		//dWorldSetAutoDisableLinearThreshold( world, 0.4f + lod*0.2f );
		//dWorldSetAutoDisableAngularThreshold( world, 0.4f + lod*0.2f );

		dWorldQuickStep( world, updateDT );
		cputimer::ticks_type t2 = cputimer::ticks();
		stats.msPhys = double(t2-t1) * cputimer::secsPerTick() * 1000.0f;
	}
}


const physics::SStats& physics::getStats()
{
	return stats;
}


#endif // PHYSICS_ODE
