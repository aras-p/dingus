#ifndef __MYDEMO_RES_H
#define __MYDEMO_RES_H


/** Possible render modes. */
enum eRenderMode {
	RM_NORMAL = 0,
	RM_CASTERSOFT,
	RMCOUNT
};

const int SZ_SHADOW = 1024;

#define RT_SHADOW "shadow"
#define RT_SHADOWBLUR "shadowBlur"
#define RT_SHADOWZ "shadowZ"


#endif
