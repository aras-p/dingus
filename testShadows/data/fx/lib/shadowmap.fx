#ifndef __SHADOWMAP_FX
#define __SHADOWMAP_FX

#include "shared.fx"
#include "structs.fx"

static inline float gShadowZ( float4 wvppos ) {
	return (wvppos.z + fShadowBias) / wvppos.w;
}

float4 psCasterMain( SPosZ i ) : COLOR {
	return i.z;
}

static inline void gShadowProj( in float4 wpos,
	in float4x4 shadowProj, in float4x4 lightVW,
	out float4 ouvz )
{
	ouvz = mul( wpos, shadowProj ).xyww;
	float4 wvpos = mul( wpos, lightVW );
	ouvz.z = wvpos.z / wvpos.w;
#ifdef DST_SHADOWS
	ouvz.z -= fShadowBias;
#endif
}

static inline float gSampleShadow( sampler2D smap, float4 uvz ) {
	float shadow;
#ifdef DST_SHADOWS
	shadow = tex2Dproj( smap, uvz ).r;
#else
	float z = tex2Dproj( smap, uvz ).r;
	shadow = z < uvz.z ? 0.0 : 1.0;
#endif
	return shadow;
}


#endif
