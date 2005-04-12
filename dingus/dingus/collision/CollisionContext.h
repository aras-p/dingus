// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __COLLISION_CONTEXT_H
#define __COLLISION_CONTEXT_H

#include "Collidable.h"

namespace dingus {


// --------------------------------------------------------------------------

class CCollisionContext {
public:
	CCollisionContext();
	~CCollisionContext();

	void	addCollidable( CCollidable& c );
	void	removeCollidable( CCollidable& c );
	bool	containsCollidable( CCollidable const& c ) const;

	void	setGlobalColListener( ICollisionListener* l ) { mGlobalColListener = l; }

	void	perform();

	CHashCollidableContainer& getWorld() { return mWorld; }

private:
	static	void	collisionCallback( void* data, dGeomID geom1, dGeomID geom2 );
	static	CCollisionContext*		mContextForCallback;

private:
	CHashCollidableContainer	mWorld;
	//CSAPCollidableContainer		mWorld;
	//CQuadtreeCollidableContainer	mWorld;
	ICollisionListener*			mGlobalColListener;
};


// --------------------------------------------------------------------------


inline void CCollisionContext::addCollidable( CCollidable& c )
{
	mWorld.add( c );
}
inline void CCollisionContext::removeCollidable( CCollidable& c )
{
	mWorld.remove( c );
}
inline bool CCollisionContext::containsCollidable( CCollidable const& c ) const
{
	return mWorld.contains( c );
}


};


#endif
