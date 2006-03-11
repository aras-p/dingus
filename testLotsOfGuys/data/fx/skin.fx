#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"

int			iBones;

texture		tNormalAO;
sampler2D	smpNormalAO = sampler_state {
	Texture = (tNormalAO);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 tolight	: COLOR0;		// skin space
	float2 uv		: TEXCOORD0;
};

SOutput vsMain0( SInput0 i ) {
	SOutput o;
	
	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin ); // world pos

	float3x3 skinT = transpose( (float3x3)skin );

	float3 tolight = vLightDir;
	tolight = mul( tolight, skinT );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	o.pos = mul( o.pos, mViewProj );

	o.uv = i.uv;

	return o;
}

half4 psMain( SOutput i ) : COLOR {
	// sample normal+AO map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;

	half occ = normalAO.a * 0.8 + 0.2;

	half diffuse = saturate( dot( normal, i.tolight.xyz*2-1 ) );

	const half3 cDiff = half3( 1.05, 1.1, 1.2 );
	half3 col = cDiff * diffuse;

	col = col * 0.4 + 0.65 * occ;

	return half4( col, 1 );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
