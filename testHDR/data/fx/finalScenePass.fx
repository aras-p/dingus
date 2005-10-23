#include "lib/hdrlib.fx"


float fMiddleGray;

sampler s0 : register(s0); // input scene RBGE
sampler s1 : register(s1); // adapted luminance


float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float3 sample = DecodeRGBE8( tex2D(s0, uv) );
	float adaptedLum = tex2D(s1, float2(0.5f, 0.5f));

	//float4 vBloom = tex2D(s2, uv);
	//float4 vStar = tex2D(s3, uv);

	// For very low light conditions, the rods will dominate the perception
	// of light, and therefore color will be desaturated and shifted
	// towards blue.
	if( false )
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
	//if( frac(uv.x*16) > 0.5 )
	if( true )
	{
		sample *= fMiddleGray / (adaptedLum + 0.001f);
		sample /= (1.0 + sample);
	}  

	// Add the star and bloom post processing effects
	//sample += g_fStarScale * vStar;
	//sample += g_fBloomScale * vBloom;

	if( uv.x > 0.95 )
		sample = adaptedLum;

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
