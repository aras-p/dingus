#ifndef __MYDEMO_RES_H
#define __MYDEMO_RES_H

#include <dingus/math/Vector4.h>
#include <dingus/math/Plane.h>


extern SVector4			gScreenFixUVs;

// --------------------------------------------------------------------------

/** Possible render modes. */
enum eRenderMode {
	RM_NORMAL = 0,
	RM_REFLECTED,		///< For reflections. Don't reflect anything, possibly use low LOD.
	RM_CASTER,			///< Caster, projected soft shadow map.
	RM_CASTERSIMPLE,	///< Caster, projected shadow map.
	RMCOUNT
};
extern const char* RMODE_PREFIX[RMCOUNT];
extern bool		gNoPixelShaders;
extern SPlane	gReflPlane;
extern float	gCharTimeBlend;
extern SVector3	gDOFParams;


enum eCubeFaces {
	CFACE_PX = 0, CFACE_NX, CFACE_PY, CFACE_NY, CFACE_PZ, CFACE_NZ, CFACE_COUNT
};


static const SVector3 ROOM_MIN = SVector3( -4.907f, 0.000f, -3.096f );
static const SVector3 ROOM_MAX = SVector3(  4.820f, 3.979f,  4.726f );
static const SVector3 ROOM_MID = (ROOM_MIN + ROOM_MAX)*0.5f;
static const SVector3 ROOM_SIZE = (ROOM_MAX - ROOM_MIN);
static const SVector3 ROOM_HSIZE = ROOM_SIZE*0.5f;

static const SVector3 LIGHT_POS_1 = SVector3( ROOM_MID.x, ROOM_MAX.y*1.5f, ROOM_MID.z );
static const SVector3 LIGHT_POS_2 = SVector3( LIGHT_POS_1.x + ROOM_SIZE.x, LIGHT_POS_1.y, LIGHT_POS_1.z );

static const double ANIM_FPS = 30.0;


// --------------------------------------------------------------------------

const int SZ_SHADOWMAP = 256;
#define RT_SHADOWMAP "shadow"
#define RT_SHADOWBLUR "shadowBlur"
#define RT_SHADOWZ "shadowZ"

const int SZ_SHADOWMAP2_BIG = 1024;
#define RT_SHADOWMAP2_BIG "shadow2B"
const int SZ_SHADOWMAP2_SM = 512;
#define RT_SHADOWMAP2_SM "shadow2S"

#define RT_SHADOWZ2 "shadowZ2"

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


static const char* WALL_TEXS[CFACE_COUNT] = { RT_REFL_PX, RT_REFL_NX, RT_REFL_PY, RT_REFL_NY, RT_REFL_PZ, RT_REFL_NZ };


#endif
