#include "stdafx.h"
#include "Physics.h"
#include <dingus/math/ODEMarshal.h>
#include <dingus/math/Plane.h>
#include <dingus/utils/CpuTimer.h>
#include <dingus/utils/Random.h>
#include <dingus/math/MathUtils.h>

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

	SVector3 force( gRandom.getFloat(-100,100), gRandom.getFloat(-10,10), gRandom.getFloat(-100,100) );
	SVector3 fpos( gRandom.getFloat(-size.x,size.x), gRandom.getFloat(-size.y,size.y), gRandom.getFloat(-size.z,size.z) );
	dBodyAddForceAtRelPos( mObject, force.x, force.y, force.z, fpos.x, fpos.y, fpos.z );
}

CPhysObject::~CPhysObject()
{
	dBodyDestroy( mObject );
	dGeomDestroy( mCollidable );
}

void CPhysObject::update( SMatrix4x4& matrix )
{
	// decrease mObject's linear velocity
	const float LVEL_FACTOR = 0.995f;
	const dReal* lvel = dBodyGetLinearVel( mObject );
	dBodySetLinearVel( mObject, lvel[0]*LVEL_FACTOR, lvel[1]*LVEL_FACTOR, lvel[2]*LVEL_FACTOR );

	// decrease mObject's angular velocity
	const float AVEL_FACTOR = 0.95f;
	const dReal* avel = dBodyGetAngularVel( mObject );
	D3DXVECTOR3 newAVel( avel[0]*AVEL_FACTOR, avel[1]*AVEL_FACTOR, avel[2]*AVEL_FACTOR );

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

	dWorldSetQuickStepNumIterations( world, 10 );

	dWorldSetAutoDisableFlag( world, 1 );
	dWorldSetAutoDisableSteps( world, 20 );
	dWorldSetAutoDisableTime( world, 0.5f );
	dWorldSetAutoDisableLinearThreshold( world, 0.12f );
	dWorldSetAutoDisableAngularThreshold( world, 0.12f );
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


void physics::update1()
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
		dWorldQuickStep( world, updateDT );
		cputimer::ticks_type t2 = cputimer::ticks();
		stats.msPhys = double(t2-t1) * cputimer::secsPerTick() * 1000.0f;
	}
}

void physics::update2()
{
}


const physics::SStats& physics::getStats()
{
	return stats;
}


#endif // PHYSICS_ODE



// --------------------------------------------------------------------------
//  NovodeX
// --------------------------------------------------------------------------


#ifdef PHYSICS_NOVODEX


namespace {

	NxPhysicsSDK*	world;
	NxScene*		scene;

	float			updateDT;

	enum eColClass {
		COLCLASS_WALL = (1<<0),
		COLCLASS_PIECESTART = 1,
		COLCLASS_PIECECOUNT = 8,
	};

	std::vector<NxShape*>	planes;


}; // namespace



CPhysObject::CPhysObject( const SMatrix4x4& matrix, const SVector3& size )
{
	// construct body and geom

    NxBodyDesc bodyDesc;
    bodyDesc.angularDamping = 0.2f;

	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set( size.x/2, size.y/2, size.z/2 );

    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack( &boxDesc );
    actorDesc.body          = &bodyDesc;
    actorDesc.density       = 1.0f / (size.x*size.y*size.z); // total mass is 1.0
	actorDesc.globalPose.setColumnMajor44( matrix );

	mObject = scene->createActor( actorDesc );

	SVector3 force( gRandom.getFloat(-100,100), gRandom.getFloat(-10,10), gRandom.getFloat(-100,100) );
	SVector3 fpos( gRandom.getFloat(-size.x,size.x), gRandom.getFloat(-size.y,size.y), gRandom.getFloat(-size.z,size.z) );
	mObject->addForceAtLocalPos(
		NxVec3(force.x,force.y,force.z), NxVec3(fpos.x,fpos.y,fpos.z), NX_FORCE );
}

CPhysObject::~CPhysObject()
{
	scene->releaseActor( *mObject );
}

void CPhysObject::update( SMatrix4x4& matrix )
{
	// get matrix
	NxMat34 pose;
	mObject->getGlobalPose( pose );
	pose.getColumnMajor44( matrix );
}



void physics::initialize( float updDT, float grav )
{
	updateDT = updDT;

	world = NxCreatePhysicsSDK( NX_PHYSICS_SDK_VERSION, NULL, NULL );

    NxMaterial defMat;
    defMat.restitution     = 0.0f;
    defMat.staticFriction  = 0.5f;
    defMat.dynamicFriction = 0.5f;
	world->setMaterialAtIndex( 0, &defMat );

    NxSceneDesc sceneDesc;
    sceneDesc.gravity.set( 0, grav, 0 );
    sceneDesc.broadPhase         = NX_BROADPHASE_COHERENT;
    sceneDesc.collisionDetection = true;

    scene = world->createScene( sceneDesc );

	scene->setTiming( updDT, 8 );
}


void physics::addPlane( const SMatrix4x4& matrix )
{
	SPlane plane( matrix.getOrigin(), matrix.getAxisZ() );

    NxPlaneShapeDesc planeDesc;
	planeDesc.normal.set( plane.a, plane.b, plane.c );
	planeDesc.d = -plane.d;

    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack( &planeDesc );
	NxActor* p = scene->createActor( actorDesc );
}


void physics::shutdown()
{
	//for( int i = 0; i < planes.size(); ++i )
	//	dGeomDestroy( planes[i] );
	//planes.clear();

	//dSpaceDestroy( space );

	world->release();
}


void physics::update1()
{
	scene->simulate( updateDT );
	scene->flushStream();
}

void physics::update2()
{
    scene->fetchResults( NX_RIGID_BODY_FINISHED, true );
}




#endif // PHYSICS_NOVODEX


