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

static const float	fShadowBias = 0.1;

sampler smpShadow : register(s4);

/*
float3	vLightPos : register(c32);
float3	vLightDir : register(c33);
float	fLightAngle : register(c34);
float4x4	mShadowProj : register(c35);
float4x4	mLightViewProj : register(c39);
*/


#endif
