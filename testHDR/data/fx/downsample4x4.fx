#include "lib/hdrlib.fx"


static const int MAX_SAMPLES = 4;
float2 vSmpOffsets[MAX_SAMPLES];


texture tBase;
sampler2D smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Point;
	AddressU = Clamp; AddressV = Clamp;
};

float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float3 sample = 0.0f;
	for( int i=0; i < MAX_SAMPLES; i++ )
	{
		float4 s = tex2D( smpBase, uv + vSmpOffsets[i] );
		sample += DecodeRGBE8( s );
	}
	return float4( sample / MAX_SAMPLES, 1 );
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
