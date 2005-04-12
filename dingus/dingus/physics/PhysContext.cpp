// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "PhysContext.h"

using namespace dingus;


CPhysContext::CPhysContext()
:	mContactCount(0)
{
	mID = dWorldCreate();
}

CPhysContext::~CPhysContext()
{
	dWorldDestroy( mID );
}

void CPhysContext::perform( float stepsize, int iterations )
{
	if( iterations > 0 )
		dWorldStepFast1( mID, stepsize, iterations );
	else {
		//dWorldStep( mID, stepsize );
		dWorldSetQuickStepNumIterations( mID, 40 );
		dWorldQuickStep( mID, stepsize );
	}
	mContacts.clear();
	mContactCount = 0;
}
