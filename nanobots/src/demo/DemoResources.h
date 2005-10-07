#ifndef __MYDEMO_RES_H
#define __MYDEMO_RES_H

#include <dingus/math/Vector4.h>
//#include <dingus/gfx/RenderableOrderedBillboards.h>

// ------------------------------------
// Shared resources

#define RID_IB_QUADSTRIP "ibqstrip"
#define RID_IB_QUADS "ibquads"

// Level minimap texture
#define RID_TEX_LEVEL "levelMmap"

// Player flags
#define RID_TEX_PLAYER1 "player1"
#define RID_TEX_PLAYER2 "player2"


// Level mesh, full
#define RID_MESH_LEVEL_FULL "levelFull"
// Level mesh, no top
#define RID_MESH_LEVEL_NOTOP "levelNoTop"
// Special points particles mesh
#define RID_MESH_POINTS "points"


// ------------------------------------


extern bool		gPlayMode;

extern SVector4	gScreenFixUVs;
extern SVector4	gFogParam;

//class CPostProcess;
//extern CPostProcess*	gPostProcess;




// --------------------------------------------------------------------------

/** Possible render modes. */
enum eRenderMode {
	RM_NORMAL = 0,	///< Normal

	RMCOUNT
};
extern const char* RMODE_PREFIX[RMCOUNT];


// --------------------------------------------------------------------------

const int GFX_DETAIL_LEVELS = 4;
const int SFX_MAX_VOLUME = 100;


struct SAppSettings {
	SAppSettings()
		: gfxDetail(GFX_DETAIL_LEVELS-1),
		musicVolume(65), soundVolume(80),
		megaZoom(30.0f), megaTilt(50.0f),
		followMode(false), showMinimap(true),
		drawAznNeedle(true), drawAznCollector(false),
		drawEntityNames(false),
		cacheLevels(true)
	{ }

	int		gfxDetail;
	int		musicVolume;
	int		soundVolume;
	float	megaZoom;
	float	megaTilt;
	bool	followMode;
	bool	showMinimap;
	bool	drawAznNeedle;
	bool	drawAznCollector;
	bool	drawEntityNames;
	bool	cacheLevels;
};



const float TURNS_PER_SEC = 6.0f;
const float SECS_PER_TURN = 1.0f / TURNS_PER_SEC;



#endif
