// HDR utilities

#ifndef __HDR_LIB_FX
#define __HDR_LIB_FX


static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

// The per-color weighting to be used for blue shift under low light.
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f); 


#define MAX_EXP (63.0)
#define MIN_EXP (-64.0)
static const float2 SCALE_BIAS = float2( MAX_EXP-MIN_EXP, MIN_EXP );
static const float2 INV_SCALE_BIAS = float2( 1.0/(MAX_EXP-MIN_EXP), -MIN_EXP/(MAX_EXP-MIN_EXP) );


// --------------------------------------------------------------------------

float4 EncodeRGBE8( in float3 rgb )
{
	float4 vEncoded;
	float maxComponent = max( max(rgb.r, rgb.g), rgb.b );
	float fexp = ceil( log2(maxComponent) );
	vEncoded.rgb = rgb / exp2(fexp);
	vEncoded.a = fexp * INV_SCALE_BIAS.x + INV_SCALE_BIAS.y;
	return vEncoded;
}

float3 DecodeRGBE8( in float4 rgbe )
{
	return rgbe.rgb * exp2( rgbe.a * SCALE_BIAS.x + SCALE_BIAS.y );
}



#endif
