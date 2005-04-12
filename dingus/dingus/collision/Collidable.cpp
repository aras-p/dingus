// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "Collidable.h"

using namespace dingus;

void CCollidable::setRotation( const SMatrix4x4& rot )
{
	dMatrix3 tr;
	odemarshal::matrixToMatrix3( rot, tr );
	dGeomSetRotation( mID, tr );
}

void CCollidable::getRotation( SMatrix4x4& rot ) const
{
	const dReal *m = dGeomGetRotation( mID );
	odemarshal::matrixFromMatrix3( m, rot );
}

void CCollidableRay::getParams( SVector3& pos, SVector3& dir ) const
{
	dVector3 p, d;
	dGeomRayGet(mID,p,d);
	odemarshal::vec3FromVector3( p, pos );
	odemarshal::vec3FromVector3( d, dir );
}

