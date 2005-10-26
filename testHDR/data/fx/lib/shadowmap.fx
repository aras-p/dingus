#ifndef __SHADOWMAP_FX
#define __SHADOWMAP_FX

#include "shared.fx"
#include "structs.fx"


static inline float gShadowZ( float4 wvppos ) {
	return wvppos.z / wvppos.w + fShadowBias;
}

float4 psCasterMain( SPosZ i ) : COLOR {
	return i.z;
}

static inline void gShadowProj( in float4 wpos,
	in float4x4 shadowProj, in float4x4 lightVW,
	out float2 ouv, out float oz )
{
	ouv = mul( wpos, shadowProj );
	float4 wvpos = mul( wpos, lightVW );
	oz = wvpos.z / wvpos.w;
}

static inline float gSampleShadow( sampler2D smap, float2 uv, float iz ) {
	float shadow;
	float z = tex2D( smap, uv );
	shadow = z < iz ? 0.0 : 1.0;
	return shadow;
}


#endif
