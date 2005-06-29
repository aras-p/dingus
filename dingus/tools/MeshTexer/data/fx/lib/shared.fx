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

// --------------------------------------------------------------------------
//  global cull params

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;

// cull mode: Solid=3 (should be default)
shared int		iFill = 3;


// --------------------------------------------------------------------------
//  light

// view space
static const float3	vLightDir = -normalize(float3(0.5,-0.5,1.0));


// --------------------------------------------------------------------------
//  mesh texer

shared	float	fTexerScale = 1.0;


#endif
