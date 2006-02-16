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

half gShadowLookup( sampler2D smap, float4 uvz, float2 offset, float2 invShadowSize ) {
	uvz.xy += offset * invShadowSize * uvz.w;
#ifdef DST_SHADOWS
	return tex2Dproj( smap, uvz ).r;
#else
	float z = tex2Dproj( smap, uvz ).r;
	return z < uvz.z ? 0.0 : 1.0;
#endif
}

#if D_SHD_FILTER == 3
static inline half gSampleShadow( sampler2D smap, float4 uvz, float2 invShadowSize, float2 position ) {
#else
static inline half gSampleShadow( sampler2D smap, float4 uvz, float2 invShadowSize ) {
#endif

#if D_SHD_FILTER == 3
	
	float2 offset = (float)(frac(position*0.5) > 0.25);
	offset.y += offset.x;
	if( offset.y > 1.1 )
		offset.y = 0;
	return 0.25 * (
		gShadowLookup( smap, uvz, offset+float2(-1.5, 0.5), invShadowSize ) +
		gShadowLookup( smap, uvz, offset+float2( 0.5, 0.5), invShadowSize ) +
		gShadowLookup( smap, uvz, offset+float2(-1.5,-1.5), invShadowSize ) +
		gShadowLookup( smap, uvz, offset+float2( 0.5,-1.5), invShadowSize )
	);

#elif D_SHD_FILTER == 2
	float sum = 0;
	float x, y;
	for( y = -1.5; y <= 1.5; y += 1.0 )
		for( x = -1.5; x <= 1.5; x += 1.0 )
			sum += gShadowLookup( smap, uvz, float2(x,y), invShadowSize );
	return sum / 16.0;
#elif D_SHD_FILTER == 1
	float sum = 0;
	float x, y;
	for( y = -1.0; y <= 1.0; y += 1.0 )
		for( x = -1.0; x <= 1.0; x += 1.0 )
			sum += gShadowLookup( smap, uvz, float2(x,y), invShadowSize );
	return sum / 9.0;
#else
	return gShadowLookup( smap, uvz, float2(0,0), float2(0,0) );
#endif
}


#endif
