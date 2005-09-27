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
	float	weight  : BLENDWEIGHT;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 color	: COLOR0;
	float2 uv		: TEXCOORD0;
	float  z		: TEXCOORD1;
};

SOutput vsMain( SInput i ) {
	SOutput o;

	float3 wpos, wn;
	o.pos.w = 1;
	gSkinning2( i.pos, i.normal*2-1, i.indices, i.weight, o.pos.xyz, wn );
	
	o.z = gCameraDepth( o.pos.xyz );

	float3 vn = mul( wn, (float3x3)mView );
	float rim = saturate( abs(vn.z) * 1.2 - 0.2 );
	o.color.rgb = 1;
	o.color.a = rim;

	o.pos = mul( o.pos, mViewProj );

	o.uv = i.uv*4 + float2(fTime*3,fTime);
	return o;
}

half4 psMain1( SOutput i ) : COLOR {
	half4 col = tex2D( smpBase, i.uv );
	col *= i.color.a * 0.20;
	return col;
}

half4 psMain2( SOutput i ) : COLOR {
	float3 COLOR = float3(0.4,0.45,1.0);
	half4 col = tex2D( smpBase, i.uv );
	return col * half4( COLOR, i.color.a * 0.5 );
}

half4 psMain3( SOutput i ) : COLOR {
	return half4( 1, 1, 1, gBluriness(i.z) );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain1();

		ColorWriteEnable = Red | Green | Blue;

		ZWriteEnable = False;
		AlphaBlendEnable = True;
		SrcBlend = Zero;
		DestBlend = InvSrcColor;
	}
	pass P1 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain2();

		SrcBlend = SrcAlpha;
		DestBlend = One;
	}
	pass P2 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain3();

		ColorWriteEnable = Alpha;

		AlphaBlendEnable = False;
	}
	RESTORE_PASS
	// TBD: save AlphaBlendEnable
	// TBD: remove duplicates
	//	ColorWriteEnable = Red | Green | Blue | Alpha;
	//	ZWriteEnable = True;
}
