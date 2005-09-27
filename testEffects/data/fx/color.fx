#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWVP;
float4		vColor;


SPosCol vsMain( SPosN i ) {
	SPosCol o;
	float3 n = mul( i.normal*2-1, (float3x3)mWorld );
	o.pos	= mul( i.pos, mWVP );

	float4 diff = saturate( dot( n, vLightDir ) ) * vColor + 0.3;
	o.color = diff;
	return o;
}



technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	RESTORE_PASS
}
