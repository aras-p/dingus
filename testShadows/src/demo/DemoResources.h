#ifndef __MYDEMO_RES_H
#define __MYDEMO_RES_H

#include <dingus/math/Vector4.h>


extern SVector4			gScreenFixUVs;


/** Possible render modes. */
enum eRenderMode {
	RM_NORMAL = 0,
	RM_ZFILL,
	RM_CASTER,
	RMCOUNT
};
extern const char* RMODE_PREFIX[RMCOUNT];


#endif
