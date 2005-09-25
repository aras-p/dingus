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

half4 psMain20( SPosCol i ) : COLOR
{
	half4 color = i.color;
	return color;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain20();

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