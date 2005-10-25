#include "lib/hdrlib.fx"


static const int MAX_SAMPLES = 4;
float2 vSmpOffsets[MAX_SAMPLES];


sampler s0 : register(s0);

half4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	half3 sample = 0.0f;
	for( int iSample = 0; iSample < MAX_SAMPLES; iSample++ )
	{
		sample += tex2D( s0, uv + vSmpOffsets[iSample] ).rgb;
	}
	sample /= MAX_SAMPLES;
	return half4( sample, 1.0f );
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
