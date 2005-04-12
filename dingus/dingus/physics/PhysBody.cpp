// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "PhysBody.h"

using namespace dingus;


void CPhysBody::setRotation( const SMatrix4x4& rot )
{
	dMatrix3 tr;
	odemarshal::matrixToMatrix3( rot, tr );
	dBodySetRotation( mID, tr );
}

void CPhysBody::getRotation( SMatrix4x4& dest ) const
{
	const dReal *m = dBodyGetRotation( mID );
	odemarshal::matrixFromMatrix3( m, dest );
}

void CPhysBody::setMass( const SPhysicsMass& mass )
{
	dMass m;
	m.mass = (float)mass.mass;
	m.c[0] = m.c[1] = m.c[2] = m.c[3] = 0.0;
	m.I[0] = mass.inertiaTensor._11;
	m.I[4] = mass.inertiaTensor._12;
	m.I[8] = mass.inertiaTensor._13;
	m.I[1] = mass.inertiaTensor._21;
	m.I[5] = mass.inertiaTensor._22;
	m.I[9] = mass.inertiaTensor._23;
	m.I[2] = mass.inertiaTensor._31;
	m.I[6] = mass.inertiaTensor._32;
	m.I[10] = mass.inertiaTensor._33;
	dBodySetMass( mID, &m );
}
