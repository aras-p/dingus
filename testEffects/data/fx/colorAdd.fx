#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWVP;
float4		vColor;


SPosCol vsMain( SPosN i ) {
	SPosCol o;
	o.pos	= mul( i.pos, mWVP );
	float3 n = mul( i.normal*2-1, (float3x3)mWorld );

	float4 diff = abs( dot( n, vLightDir ) ) * vColor + 0.3;
	o.color = diff;
	o.color.a = 0.5;
	return o;
}

float4 psMain( SPosCol i ) : COLOR
{
	return i.color;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = One;

		ZWriteEnable = False;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = One;

		ZWriteEnable = False;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
