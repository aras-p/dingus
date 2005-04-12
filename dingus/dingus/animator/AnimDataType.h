// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIM_DATA_TYPE_H
#define __ANIM_DATA_TYPE_H

namespace dingus {

// NOTE: don't change the constants! exported files might have them!

/** Animation type in a data file (and nowhere else). */
enum eAnimDataFileType {
	ANIMTYPE_CUSTOM	= 0,
	ANIMTYPE_FLT	= 1,	// 1 float
	ANIMTYPE_INT	= 2,	// 1 int
	ANIMTYPE_VEC3	= 3,	// 3 floats
	ANIMTYPE_QUAT	= 4,	// 4 floats, to be treated as quaternion
	ANIMTYPE_COLOR	= 5,	// 1 int (d3dcolor), to be treated as vector4
	ANIMTYPE_FLT_H	= 6,	// 1 16bit float
	ANIMTYPE_VEC3_H	= 7,	// 3 16bit floats
	ANIMTYPE_QUAT_H	= 8,	// 4 16bit floats - quaternion
};

};

#endif
