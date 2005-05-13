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
shared float3		vEye;

// --------------------------------------------------------------------------
//  global cull params

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;

// cull mode: Solid=3 (should be default)
shared int		iFill = 3;


// --------------------------------------------------------------------------
//  shadow maps

static const float fShadowRangeScale = 10.0;
static const float fShadowBias = 0.003;

shared float4x4	mViewTexProj;

shared float4x4	mShadowProj;
shared float4x4	mShadowProj2;

shared float4x4	mLightViewProj;

shared float3	vLightPos;


shared texture	tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};

shared texture	tShadow2;
sampler2D	smpShadow2 = sampler_state {
	Texture = (tShadow2);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};

static const int SHADOW_MAP_SIZE = 256;
static const int SHADOW_MAP_SIZE2 = 512;


// --------------------------------------------------------------------------
//  others


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
