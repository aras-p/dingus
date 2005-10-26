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
// shadows

static const float	fShadowBias = 0.0015;
shared float	fLightIntensity;
shared float3	vLightDir;
shared float4x4	mShadowProj;
shared float4x4	mLightViewProj;



// --------------------------------------------------------------------------
//  others

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;



#endif
