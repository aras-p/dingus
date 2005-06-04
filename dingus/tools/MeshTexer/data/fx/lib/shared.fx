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

// --------------------------------------------------------------------------
//  global cull params

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;

// cull mode: Solid=3 (should be default)
shared int		iFill = 3;


// --------------------------------------------------------------------------
//  light

shared float3	vLightPos;


// --------------------------------------------------------------------------
//  mesh texer

shared	float	fTexerScaleDiff = 1.0;
shared	float	fTexerScaleGloss = 1.0;

shared	float	fTexerGlossBias = 0.0;
shared	float	fTexerGlossScale = 1.0;


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
