#ifndef __SHADOWMAP_FX
#define __SHADOWMAP_FX

#include "shared.fx"
#include "structs.fx"

#ifdef DST_SHADOWS
half4 psCasterMain() : COLOR {
	return 0;
}
#else
static inline float gShadowZ( float4 wvppos ) {
	return (wvppos.z + fShadowBias) / wvppos.w;
}
float4 psCasterMain( SPosZ i ) : COLOR {
	return i.z;
}
#endif

static inline void gShadowProj( in float4 wpos,
	in float4x4 shadowProj,
	out float4 ouvz )
{
	ouvz = mul( wpos, shadowProj );
#ifndef DST_SHADOWS
	ouvz.z /= ouvz.w;
#endif
}

static inline half gSampleShadow( sampler2D smap, float4 uvz ) {
#ifdef DST_SHADOWS
	return tex2Dproj( smap, uvz ).r;
#else
	float z = tex2Dproj( smap, uvz ).r;
	return z < uvz.z ? 0.0 : 1.0;
#endif
}


#endif
