#include "lib/hdrlib.fx"


float fDeltaTime;

sampler s0 : register(s0);
sampler s1 : register(s1);


float4 psMain( in float2 uv : TEXCOORD0 ) : COLOR
{
	float lumAdapted = tex2D(s0, float2(0.5f, 0.5f));
	float lumCurr = tex2D(s1, float2(0.5f, 0.5f));

	// The user's adapted luminance level is simulated by closing the gap
	// between adapted luminance and current luminance by 2% every frame,
	// based on a 30 fps rate. This is not an accurate model of human
	// adaptation, which can take longer than half an hour.
	float lumNew = lumAdapted + (lumCurr - lumAdapted) * ( 1 - pow( 0.98f, 30 * fDeltaTime ) );
	return float4(lumNew, lumNew, lumNew, 1.0f);
}

technique tec20
{
	pass P0 {
		VertexShader = NULL;
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
