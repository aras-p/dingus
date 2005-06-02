// global parameters are here

#ifndef __SHARED_FX
#define __SHARED_FX

#include "defines.fx"


// --------------------------------------------------------------------------
// time

shared float	fTime;

// --------------------------------------------------------------------------
// camera

shared float4x4	mView;
shared float4x4	mProjection;
shared float4x4	mViewProj;
shared float3	vEye;
shared float3	vCameraX;
shared float3	vCameraY;

// --------------------------------------------------------------------------
//  others

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;


// fog: x=start, y=end, z=1/(end-start)
shared float4	vFog;
// fog color
shared float4	vFogColor;


// --------------------------------------------------------------------------


const static float4x4 mIdentity = float4x4(
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1 );
const static float4x4 mSphereMap = float4x4(
	0.4,0,  0, 0,
	0, -0.4,0, 0,
	0,  0.0,0, 0,
	0.5,0.5,0, 0 );


#endif
