#ifndef __DOF_FX
#define __DOF_FX

#include "shared.fx"

/**
 *  Given world space position, returns
 *  view space depth.
 */
float	gCameraDepth( float3 wpos )
{
	float3 vpos = mul( float4(wpos,1), mView );
	return vpos.z;
}


/**
 *  Given object space position and world*view matrix, returns
 *  view space depth.
 */
float	gCameraDepth( float4 pos, float4x4 viewProj )
{
	float3 vpos = mul( pos, viewProj );
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
		fromFocal *= 2;
	return abs( fromFocal ) * vDOF.y;
}

#endif
