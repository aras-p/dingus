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

#ifndef DST_SHADOWS
static const float	fShadowBias = 0.1;
#endif

sampler smpShadow : register(s4);


#endif
