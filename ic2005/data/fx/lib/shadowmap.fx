#ifndef __SHADOWMAP_FX
#define __SHADOWMAP_FX

#include "shared.fx"
#include "structs.fx"


static inline float gShadowZ( float4 wvppos ) {
	//return wvppos.z * fShadowRangeScale + fShadowBias;
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
	//oz = mul( wpos, lightVW ).z * fShadowRangeScale;
	float4 wvpos = mul( wpos, lightVW );
	oz = wvpos.z / wvpos.w;
}

static inline float gSampleShadow( sampler2D smap, float2 uv, float iz ) {
	float shadow;
	float z = tex2D( smap, uv );
	shadow = z < iz ? 0.0 : 1.0;
	return shadow;
}


// jittered sample offset table for 1k shadow maps
static const float4 gSampleOffsets[] = {
	float4( -0.000692, -0.000868, -0.002347, 0.000450 ),
	float4( 0.000773, -0.002042, -0.001592, 0.001880 ),
	float4( -0.001208, -0.001198, -0.000425, -0.000915 ),
	float4( -0.000050, 0.000105, -0.000753, 0.001719 ),
	float4( -0.001855, -0.000004, 0.001140, -0.001212 ),
	float4(  0.000684, 0.000273, 0.000177, 0.000647 ),
	float4( -0.001448, 0.002095, 0.000811, 0.000421 ),
	float4(  0.000542, 0.001491, 0.000537, 0.002367 )
};

static inline float gSampleShadow16( sampler2D smap, float2 uv, float iz ) {
	int i;
	float shadow = 0;
	float4 row1;
	float4 row2;
	for( i = 0; i < 4; ++i ) {
		row1[i] = tex2D( smap, uv + gSampleOffsets[i].xyzw );
		row2[i] = tex2D( smap, uv + gSampleOffsets[i].zwxy );
	}
	row1 = iz - row1;
	row2 = iz - row2;
	float4 s1 = row1 < 0 ? float4(1,1,1,1) : float4(0,0,0,0);
	float4 s2 = row2 < 0 ? float4(1,1,1,1) : float4(0,0,0,0);
	shadow = dot( s1, 1.0/16.0 ) + dot( s2, 1.0/16.0 );

	for( i = 0; i < 4; ++i ) {
		row1[i] = tex2D( smap, uv + gSampleOffsets[i+4].xyzw );
		row2[i] = tex2D( smap, uv + gSampleOffsets[i+4].zwxy );
	}
	row1 = iz - row1;
	row2 = iz - row2;
	s1 = row1 < 0 ? float4(1,1,1,1) : float4(0,0,0,0);
	s2 = row2 < 0 ? float4(1,1,1,1) : float4(0,0,0,0);
	shadow += dot( s1, 1.0/16.0 ) + dot( s2, 1.0/16.0 );

	return shadow;
}



#endif
