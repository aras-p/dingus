// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PHYSICS_JOINT_GROUP_H
#define __PHYSICS_JOINT_GROUP_H

#include "../math/ODEMarshal.h"

namespace dingus {



// --------------------------------------------------------------------------

class CPhysJointGroup : public boost::noncopyable {
public:
	CPhysJointGroup() { mID = dJointGroupCreate(0); }
	~CPhysJointGroup() { dJointGroupDestroy(mID); }
	void create() {
		if(mID) dJointGroupDestroy (mID);
		mID = dJointGroupCreate(0);
	}
	
	TPhysJointGroupID getID() const { return mID; }
	operator TPhysJointGroupID() const { return mID; }
	
	void clear() { dJointGroupEmpty(mID); }

private:
	TPhysJointGroupID mID;
};


}; // namespace

#endif
