// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIM_TIME_H
#define __ANIM_TIME_H

namespace dingus {


// kind of HACK
inline double anim_time() {
	return CSystemTimer::getInstance().getTimeS();
};


};


#endif
