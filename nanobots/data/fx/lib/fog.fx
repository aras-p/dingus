#ifndef __FOG_FX
#define __FOG_FX

#include "shared.fx"

static inline float gFogWV( float4 pos, float4x4 wv ) {
	float3 vpos = mul( pos, wv );
	//float d = length(vpos);
	float d = vpos.z;
	return (vFog.y - d) * vFog.z;
}

static inline float gFog( float4 pos ) {
	//float d = length(pos.xyz-vEye);
	float3 vpos = mul( pos, mView );
	float d = vpos.z;
	return (vFog.y - d) * vFog.z;
}



#endif
