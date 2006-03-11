// global parameters are here

#ifndef __SHARED_FX
#define __SHARED_FX

// --------------------------------------------------------------------------
// camera

shared float4x4	mView;
shared float4x4	mProjection;
shared float4x4	mViewProj;
shared float3	vEye;
shared float3	vCameraX;
shared float3	vCameraY;

// --------------------------------------------------------------------------
// light and shadows

const float3 vLightDir = normalize(float3(-0.2,1,0.3));

#endif
