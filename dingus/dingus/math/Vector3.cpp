// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Vector3.h"
#include "Constants.h"
#include "FPU.h"

using namespace dingus;


// Pretty straight from ODE sources :)
void SVector3::planeSpace( SVector3& p, SVector3& q ) const
{
	if( fabsf( z ) > SQRT12 ) {
		// choose p in y-z plane
		float a = y*y + z*z;
		float k = invSqrt( a );
		p.x = 0;
		p.y = -z*k;
		p.z = y*k;
		// set q = n cross p
		q.x = a*k;
		q.y = -x*p.z;
		q.z = x*p.y;
	} else {
		// choose p in x-y plane
		float a = x*x + y*y;
		float k = invSqrt(a);
		p.x = -y*k;
		p.y = x*k;
		p.z = 0;
		// set q = n x p
		q.x = -z*p.y;
		q.y = z*p.x;
		q.z = a*k;
	}
}
