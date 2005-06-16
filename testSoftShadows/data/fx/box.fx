#include "lib/shared.fx"
#include "lib/structs.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Anisotropic; MinFilter = Anisotropic; MipFilter = Linear;
	MaxAnisotropy = 2; // 8
	AddressU = Wrap; AddressV = Wrap;
};

texture		tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


struct SOutput11 {
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
	float4 uvs : TEXCOORD1;
	half4 color : COLOR0;
};


SOutput11 vsMain( SPosNTex i ) {
	SOutput11 o;
	o.pos = mul( i.pos, mViewProj );
	float diff = max( 0, dot( i.normal, vLightDir ) ) * 0.6;
	o.color = diff + 0.5;
	o.uv = i.uv;
	float4 vpos = mul( i.pos, mView );
	o.uvs = mul( vpos, mShadowProj );
	return o;
}

half4 psMain( SOutput11 i ) : COLOR
{
	//half4 col = tex2D( smpBase, i.uv );
	half4 col = 1;
	half shadow = tex2Dproj( smpShadow, i.uvs ).g;
	return col * i.color * shadow;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
		Texture[1] = NULL;
	}
}
