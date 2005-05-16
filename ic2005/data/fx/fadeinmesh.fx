#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

float4x4	mWorld;
float4x4	mWVP;
float3		vNormal;

texture		tAlpha;
sampler2D	smpAlpha = sampler_state {
	Texture = (tAlpha);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};

texture		tRefl;
sampler2D	smpRefl = sampler_state {
	Texture = (tRefl);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


struct SOutput {
	float4	pos		: POSITION;
	half4	color	: COLOR;
	float2	uv		: TEXCOORD0;
	float4	uvp		: TEXCOORD1;
};


SOutput vsMain( SPosNTex i ) {
	SOutput o;
	float3 wpos = mul( i.pos, mWorld );
	o.pos = mul( i.pos, mWVP );
	o.color = gWallLight( wpos, vNormal );
	o.uv.x = 1-i.uv.x;
	o.uv.y = 1-i.uv.y;
	o.uvp = mul( float4(wpos,1), mViewTexProj );
	return o;
}

half4 psMain( SOutput i ) : COLOR {
	half a = tex2D( smpAlpha, i.uv ).a;

	half3 col = i.color.rgb;
	col += tex2Dproj( smpRefl, i.uvp ).rgb * 0.15;
	return half4(col,a);
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = False;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
