#ifndef __MYDEMO_RES_H
#define __MYDEMO_RES_H

#include <dingus/math/Vector4.h>


extern SVector4			gScreenFixUVs;

// --------------------------------------------------------------------------

/** Possible render modes. */
enum eRenderMode {
	RM_NORMAL = 0,
	RM_REFLECTED,		///< For reflections. Don't reflect anything, possibly use low LOD.
	RM_CASTERSIMPLE,	///< Caster projected shadow map.
	RMCOUNT
};
extern const char* RMODE_PREFIX[RMCOUNT];


enum eCubeFaces {
	CFACE_PX = 0, CFACE_NX, CFACE_PY, CFACE_NY, CFACE_PZ, CFACE_NZ, CFACE_COUNT
};


// --------------------------------------------------------------------------

/*
const int SZ_SHADOW_AO = 256;
#define RT_SHADOW_AO1 "shadowAO1"
#define RT_SHADOW_AO2 "shadowAO2"
#define RT_SHADOW_AO3 "shadowAO3"
#define RT_SHADOW_AO4 "shadowAO4"
#define RT_SHADOW_AOZ "shadowAOZ"
*/

const int SZ_SHADOWMAP = 256;
#define RT_SHADOWMAP "shadow"
#define RT_SHADOWBLUR "shadowBlur"
#define RT_SHADOWZ "shadowZ"

//
// walls reflections

const float SZ_REFLRT_REL = 0.5f;
const float SZ_REFLBLUR_REL = 0.25f;

// render target and z/stencil
#define RT_REFLRT "reflRT"
#define RT_REFLZ "reflZ"
// small temp ones for blurring
#define RT_REFL_TMP1 "reflTmp1"
#define RT_REFL_TMP2 "reflTmp2"
// small final one textures
#define RT_REFL_PX "reflpx"
#define RT_REFL_NX "reflnx"
#define RT_REFL_PY "reflpy"
#define RT_REFL_NY "reflny"
#define RT_REFL_PZ "reflpz"
#define RT_REFL_NZ "reflnz"


#endif
