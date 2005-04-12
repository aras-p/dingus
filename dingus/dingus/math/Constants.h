// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATH_CONSTANTS_H
#define __MATH_CONSTANTS_H

#include <float.h>


namespace dingus {


/** PI constant. */
const float PY = 3.1415926535897932384626433832795029f;
/** Sqrt(1/2) constant. */
const float SQRT12 = 0.7071067811865475244008443621048490f;


/** Maximum float value. */
const float DINGUS_MAX_FLOAT = FLT_MAX;
/** Minimum positive float value. */
const float DINGUS_MIN_FLOAT = FLT_MIN;

/** Big float value. */
const float DINGUS_BIG_FLOAT = 1.0e30f;
/** Very small float value. */
const float DINGUS_SMALL_FLOAT = FLT_EPSILON;


};


#endif
