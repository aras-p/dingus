// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PRIMITIVES_H
#define __PRIMITIVES_H


#include "Vector3.h"

namespace dingus {


struct SRay {
	SVector3	origin;
	SVector3	direction;
};


struct SSphere {
	SVector3	origin;
	float		radius;
};


}; // namespace

#endif
