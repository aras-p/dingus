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


static const float3 vLightDir = -normalize( float3(0.4, -1.0, 0.2) );


// shadow mapping

shared float4x4	mViewTexProj;
shared float4x4 mShadowProj;

#define SHADOW_MAP_SIZE 1024.0



#endif
