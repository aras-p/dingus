#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWVP;
float4		vColor;


SPosCol vsMain( SPosN i ) {
	SPosCol o;
	i.normal = i.normal*2-1;

	o.pos = mul( i.pos, mWVP );
	float3 n = mul( i.normal, (float3x3)mWorld );
	o.color.xyz = n*0.5+0.5;
	o.color.w = 1;
	return o;
}

half4 psMain( SPosCol i ) : COLOR
{
	return i.color;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();

		AlphaBlendEnable = True;
		SrcBlend = One;
		DestBlend = One;

		ColorWriteEnable = Red | Green;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ColorWriteEnable = Red | Green | Blue | Alpha;
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		AlphaBlendEnable = True;
		SrcBlend = One;
		DestBlend = One;

		ColorWriteEnable = Red | Green;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ColorWriteEnable = Red | Green | Blue | Alpha;
	}
}
