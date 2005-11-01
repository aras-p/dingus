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
// HDR

shared float	fMiddleGray;
shared float	fEnvIntensity;

// environment SH
shared float4 vSHAr;
shared float4 vSHAg;
shared float4 vSHAb;
shared float4 vSHBr;
shared float4 vSHBg;
shared float4 vSHBb;
shared float4 vSHC;


static inline float3 evalSHEnv( float4 n )
{
	float3 x1, x2, x3;

	// Linear + constant polynomial terms
	x1.r = dot(vSHAr,n);
	x1.g = dot(vSHAg,n);
	x1.b = dot(vSHAb,n);

	// 4 of the quadratic polynomials
	float4 vB = n.xyzz * n.yzzx;
	x2.r = dot(vSHBr,vB);
	x2.g = dot(vSHBg,vB);
	x2.b = dot(vSHBb,vB);

	// Final quadratic polynomial
	float vC = n.x*n.x - n.y*n.y;
	x3 = vSHC.rgb * vC;

	return x1 + x2 + x3;
}




// --------------------------------------------------------------------------
//  others

// cull mode: CW=2 (should be default), CCW=3 (for reflected, etc.)
shared int		iCull = 2;



#endif
