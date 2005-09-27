#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWVP;
float4		vColor;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};


SPosColTex vsMain( SPosN i ) {
	SPosColTex o;
	float3 n = mul( i.normal*2-1, (float3x3)mWorld );
	o.pos	= mul( i.pos, mWVP );

	float4 diff = saturate( dot( n, vLightDir ) ) * vColor + 0.3;
	o.color = diff;

	o.uv = i.pos.xy*0.2;

	return o;
}

float4 psMain( SPosColTex i ) : COLOR
{
	return tex2D( smpBase, i.uv ) * i.color;
}



technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
	}
	pass PLast {
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		Sampler[0]	 = (smpBase);

		ColorOp[0]	 = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0]	 = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	pass PLast {
	}
}
