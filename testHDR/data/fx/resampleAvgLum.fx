#include "lib/hdrlib.fx"


static const int MAX_SAMPLES = 16;
float2 vSmpOffsets[MAX_SAMPLES];


sampler s0 : register(s0);


float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float sample = 0.0f;
	for( int i=0; i < MAX_SAMPLES; i++ )
	{
		sample += tex2D( s0, uv + vSmpOffsets[i] );
	}
	sample /= MAX_SAMPLES;
	return float4( sample, sample, sample, 1 );
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_3_0 psMain(); // TBD: fixme for ps2.0!
	}
	RESTORE_PASS
}
