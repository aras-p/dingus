// global parameters are here

#ifndef __SHARED_FX
#define __SHARED_FX


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


static const float3 vLightDir = -normalize( float3(0.4, -1.0, 0.8) );


#endif
