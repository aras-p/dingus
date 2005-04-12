// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIM_TIME_H
#define __ANIM_TIME_H

namespace dingus {


// kind of HACK
inline float anim_time() {
	return (float)CSystemTimer::getInstance().getTimeS();
};


};


#endif
