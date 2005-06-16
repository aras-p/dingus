#ifndef __SHADOWMAP_FX
#define __SHADOWMAP_FX

#include "shared.fx"
#include "structs.fx"

static inline float gShadowZ( float4 wvppos ) {
	return wvppos.z * fShadowRangeScale + fShadowBias;
}

float4 psCasterMain( SPosZ i ) : COLOR {
	return i.z;
}

static inline void gShadowProj( in float4 wpos, out float2 ouv, out float oz ) {
	ouv = mul( wpos, mShadowProj );
	oz = mul( wpos, mLightViewProj ).z * fShadowRangeScale;
}

#endif
