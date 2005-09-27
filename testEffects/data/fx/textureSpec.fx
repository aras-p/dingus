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

struct SOutput {
	float4 pos	: POSITION;
	float2 uv	: TEXCOORD0;
	half3  n	: TEXCOORD1;
	half3  h	: TEXCOORD2;
};


SOutput vsMain( SPosN i ) {
	SOutput o;
	float3 n = mul( i.normal*2-1, (float3x3)mWorld );
	o.n = n;

	float3 wpos = mul( i.pos, mWorld );
	float3 toview = normalize( vEye - wpos );
	float3 h = normalize( vLightDir + toview );
	o.h = h;

	o.pos	= mul( i.pos, mWVP );

	o.uv = i.pos.xy*0.2;

	return o;
}

half4 psMain( SOutput i ) : COLOR
{
	half4 col = tex2D( smpBase, i.uv );

	i.n = normalize( i.n );
	i.h = normalize( i.h );
	
	half diff = saturate( dot( i.n, vLightDir ) );
	half spec = pow( saturate(dot( i.n, i.h )), 16 );

	return diff * col.g + spec * col.r;
}



technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		Sampler[0]	 = (smpBase);

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Texture;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	RESTORE_PASS
}
