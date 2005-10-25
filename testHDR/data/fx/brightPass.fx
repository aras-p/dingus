#include "lib/hdrlib.fx"


static const float  BRIGHT_PASS_THRESHOLD  = 2.0f; // default 5.0f
static const float  BRIGHT_PASS_OFFSET     = 5.0f; // default 10.0f


float fMiddleGray;

sampler s0 : register(s0); // input scene float
sampler s1 : register(s1); // adapted luminance


half4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float3 sample = tex2D(s0, uv);
	float adaptedLum = tex2D(s1, float2(0.5f, 0.5f));

	// determine pixel's value after tone-mapping
	sample *= fMiddleGray/(adaptedLum + 0.001f);
	
	// subtract out dark pixels
	sample -= BRIGHT_PASS_THRESHOLD;
	sample = max(sample, 0.0f);
	
	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	sample /= (BRIGHT_PASS_OFFSET+sample);
    
	return half4(sample,1);
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
