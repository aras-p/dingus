
#if D_SHADOWS==1
#define WALL_SHADOW
	#if D_REFLECTIONS==1
	#define WALL_SH2REFL
	#endif
#endif

#if D_REFLECTIONS==1
#define WALL_REFL
#endif

#include "walls.fx"
