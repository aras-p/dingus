#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/dof.fx"


int			iBones;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};


struct SInput {
	float4	pos		: POSITION;
	float3	weight  : BLENDWEIGHT;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
};

SOutput vsMain( SInput i ) {
	SOutput o;

	float3 wpos, wn;
	o.pos.w = 1;
	gSkinning( i.pos, i.normal*2-1, i.indices, i.weight, o.pos.xyz, wn, 4 );
	
	//float3 vn = mul( wn, (float3x3)mView );
	//float rim = saturate( abs(vn.z) * 1.2 - 0.2 );
	//o.color.rgb = 1;
	//o.color.a = rim;

	o.color = 1;

	o.pos = mul( o.pos, mViewProj );

	o.uv = i.uv*4 + float2(fTime*0.5,fTime*0.1);
	return o;
}

half4 psMain( SOutput i ) : COLOR {
	float3 COLOR = float3(0.4,0.45,1.0);
	half4 col = tex2D( smpBase, i.uv );
	return col * half4( COLOR, i.color.a * 0.5 );
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();

		ColorWriteEnable = Red | Green | Blue;

		ZWriteEnable = False;
		//AlphaBlendEnable = True;
		SrcBlend = Zero;
		DestBlend = InvSrcColor;
	}
	pass PLast {
		ColorWriteEnable = Red | Green | Blue | Alpha;
		ZWriteEnable = True;
		AlphaBlendEnable = False;
	}
}
