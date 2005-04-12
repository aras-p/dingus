// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __COLLISION_LISTENER_H
#define __COLLISION_LISTENER_H

namespace dingus {

class CCollidable;


// --------------------------------------------------------------------------

class ICollisionListener {
public:
	virtual ~ICollisionListener() = 0 { }
	virtual void onCollide( CCollidable& me, CCollidable& him ) = 0;
};

	
}; // namespace


#endif
