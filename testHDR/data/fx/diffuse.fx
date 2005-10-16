#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWVP;


SPosCol vsMain( SPosN i ) {
	SPosCol o;
	float3 n = mul( i.normal*2-1, (float3x3)mWorld );
	o.pos	= mul( i.pos, mWVP );
	float4 diff = saturate( dot( n, vLightDir ) ) + 0.3;
	o.color = diff;
	return o;
}


half4 psMain( SPosCol i ) : COLOR
{
	return i.color;
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
