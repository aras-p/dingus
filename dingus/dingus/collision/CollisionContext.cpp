// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "CollisionContext.h"

using namespace dingus;


CCollisionContext* CCollisionContext::mContextForCallback = 0;

CCollisionContext::CCollisionContext()
:	mWorld(NULL),
	mGlobalColListener(NULL)
{
}


CCollisionContext::~CCollisionContext()
{
}

void CCollisionContext::perform()
{
	mContextForCallback = this;
	dSpaceCollide( mWorld, NULL, &collisionCallback );
}

void CCollisionContext::collisionCallback( void* data, dGeomID geom1, dGeomID geom2 )
{
	assert( mContextForCallback );
	CCollisionContext& ctx = *mContextForCallback;

	if( dGeomIsSpace(geom1) || dGeomIsSpace(geom2) ) {
		// space with something
		dSpaceCollide2( geom1, geom2, data, &collisionCallback );

		// NOTE: I think it's not needed - child spaces won't collide
		// internal geoms
		//if( dGeomIsSpace(geom1) ) dSpaceCollide( geom1, data, &collisionCallback );
		//if( dGeomIsSpace(geom2) ) dSpaceCollide( geom2, data, &collisionCallback );
	} else {
		// fetch CCollidables from geoms
		CCollidable* col1 = reinterpret_cast<CCollidable*>( dGeomGetData(geom1) );
		CCollidable* col2 = reinterpret_cast<CCollidable*>( dGeomGetData(geom2) );
		assert( col1 && col2 );
		col1->notifyListeners( *col2 );
		col2->notifyListeners( *col1 );
		if( ctx.mGlobalColListener )
			ctx.mGlobalColListener->onCollide( *col1, *col2 );
	}
}
