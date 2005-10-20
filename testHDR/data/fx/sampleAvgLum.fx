#include "lib/hdrlib.fx"


static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

static const int MAX_SAMPLES = 9;
float2 vSmpOffsets[MAX_SAMPLES];


texture tBase;
sampler2D smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Point;
	AddressU = Clamp; AddressV = Clamp;
};

float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float3 vSample = 0.0f;
	float  fLogLumSum = 0.0f;
	for( int iSample = 0; iSample < MAX_SAMPLES; iSample++ )
	{
		vSample = tex2D( smpBase, uv + vSmpOffsets[iSample] ).rgb;
		fLogLumSum += log( dot(vSample, LUMINANCE_VECTOR)+0.0001f );
	}
	fLogLumSum /= MAX_SAMPLES;
	return float4( fLogLumSum, fLogLumSum, fLogLumSum, 1.0f );
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
