#include "lib/hdrlib.fx"


float fMiddleGray;
float fBloomScale = 1.0f;

sampler s0 : register(s0); // input scene RBGE
sampler s1 : register(s1); // adapted luminance
sampler s2 : register(s2); // bloom


float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float3 sample = DecodeRGBE8( tex2D(s0, uv) );
	float adaptedLum = tex2D(s1, float2(0.5f, 0.5f));

	half4 bloom = tex2D(s2, uv);

	// For very low light conditions, the rods will dominate the perception
	// of light, and therefore color will be desaturated and shifted
	// towards blue.
	if( true )
	{
		// Define a linear blending from -1.5 to 2.6 (log scale) which
		// determines the lerp amount for blue shift
		float blueShiftCoeff = 1.0f - (adaptedLum + 1.5)/4.1;
		blueShiftCoeff = saturate(blueShiftCoeff);

		// Lerp between current color and blue, desaturated copy
		float3 rodColor = dot( sample, LUMINANCE_VECTOR ) * BLUE_SHIFT_VECTOR;
		sample = lerp( sample, rodColor, blueShiftCoeff );
	}

	// Map the high range of color values into a range appropriate for
	// display, taking into account the user's adaptation level, and selected
	// value for middle gray
	if( true )
	{

		sample *= fMiddleGray / (adaptedLum + 0.001f);
		//const float LUM_WHITE = 1.5;
		//sample *= (1.0 + sample/LUM_WHITE);
		sample /= (1.0 + sample);
	}  

	// add bloom
	sample += bloom * fBloomScale;

	return float4( sample, 1 );
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
		SRGBWriteEnable = True;
	}
	RESTORE_PASS
}
