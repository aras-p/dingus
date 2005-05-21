#ifndef __DOF_FX
#define __DOF_FX

#include "shared.fx"

/**
 *  Given world space position, returns
 *  view space depth.
 */
float	gCameraDepth( float3 wpos )
{
	float3 vpos = mul( wpos, mView );
	return vpos.z;
}


/**
 *  Given view space depth, compute bluriness
 *  factor from DOF parameters.
 */
half 	gBluriness( half depth )
{
	half fromFocal = depth - vDOF.x;
	if( fromFocal < 0 )
		fromFocal *= 4;
	return abs( fromFocal ) * vDOF.y + vDOF.z;
}

#endif
