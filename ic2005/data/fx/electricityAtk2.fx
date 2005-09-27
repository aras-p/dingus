#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/dof.fx"


int			iBones;


texture		tEnv1;
sampler2D	smpEnv1 = sampler_state {
	Texture = (tEnv1);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};
texture		tEnv2;
sampler2D	smpEnv2 = sampler_state {
	Texture = (tEnv2);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};
texture		tScroll;
sampler2D	smpScroll = sampler_state {
	Texture = (tScroll);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};


struct SInput {
	float4	pos		: POSITION;
	float	weight  : BLENDWEIGHT;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput1 {
	float4 pos		: POSITION;
	float2 uv[2]	: TEXCOORD0;
};

SOutput1 vsMain1( SInput i ) {
	SOutput1 o;

	float3 wpos, wn;
	o.pos.w = 1;
	gSkinning2( i.pos, i.normal*2-1, i.indices, i.weight, o.pos.xyz, wn );

	float3 vn = mul( wn, (float3x3)mView );
	o.uv[1] = vn.xy*0.35+0.5;
	
	o.pos = mul( o.pos, mViewProj );

	o.uv[0].x = i.uv.x * 0.05 + fTime*0.12;
	o.uv[0].y = i.uv.y * 0.5 + fTime*0.2;
	return o;
}

half4 psMain1( SOutput1 i ) : COLOR {
	half4 cenv1 = tex2D( smpEnv1, i.uv[1] );
	half4 cenv2 = tex2D( smpEnv2, i.uv[1] );
	half4 cscrl = tex2D( smpScroll, i.uv[0] );

	half4 c = lerp( cenv1, cenv2, cscrl.g );
	c.a *= 0.9;

	return c;
}



struct SOutput2 {
	float4 pos		: POSITION;
	float  z		: TEXCOORD0;
};

SOutput2 vsMain2( SInput i ) {
	SOutput2 o;

	float3 wpos;
	o.pos.w = 1;
	gSkinningPos2( i.pos, i.indices, i.weight, o.pos.xyz );

	o.z = gCameraDepth( o.pos.xyz );

	o.pos = mul( o.pos, mViewProj );

	return o;
}

half4 psMain2( SOutput2 i ) : COLOR {
	return gBluriness(i.z);
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain1();
		PixelShader = compile ps_2_0 psMain1();

		ColorWriteEnable = Red | Green | Blue;

		ZWriteEnable = False;
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		CullMode = None;
	}
	pass P1 {
		VertexShader = compile vs_1_1 vsMain2();
		PixelShader = compile ps_2_0 psMain2();

		ColorWriteEnable = Alpha;

		CullMode = <iCull>;

		ZWriteEnable = False;
		AlphaBlendEnable = False;
	}
	RESTORE_PASS
}
