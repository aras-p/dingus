#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/meshtexer.fx"

float4x4	mWorld;
float4x4	mWorldView;
float4x4	mWVP;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tNormalAO;
sampler2D	smpNormalAO = sampler_state {
	Texture = (tNormalAO);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 tolight	: COLOR0;		// object space
	float2 uv		: TEXCOORD0;
	float3 opos		: TEXCOORD1;
	float3 halfang	: TEXCOORD2;	// object space
};


SOutput vsMain( SPosTex i ) {
	SOutput o;

	float3 wpos = mul( i.pos, mWorld );
	o.opos = wpos;
	o.pos = mul( i.pos, mWVP );

	float3x3 wT = transpose( (float3x3)mWorld );
	float3x3 wvT = transpose( (float3x3)mWorldView );
	
	float3 tolight = mul( vLightDir, wvT );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	float3 toview = normalize( vEye - wpos );
	toview = mul( toview, wT );
	o.halfang = normalize( tolight + toview );

	o.uv = i.uv;

	return o;
}


half4 psMain( SOutput i ) : COLOR {
	// sample normal+AO map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;
	normal = normalize( normal );

	half amb = 0.1;
	half ambBias = 0.2;
	half ambMul = 0.8;
	half occ = normalAO.a * ambMul;

	// calc lighting
	half diffuse = saturate( dot( normal, i.tolight.xyz*2-1 ) ) * occ + ambBias;
	float spec = pow( saturate( dot( normal, i.halfang ) ), 16 );

	half3 scales = abs(normal);
	scales *= scales;

	// construct diffuse map
	half3 cDiff = gMapDiffuse( i.opos, smpBase, scales );

	half3 cSpec = cDiff * 0.3;

	half3 col = cDiff * diffuse + cSpec * spec + amb;

	return half4( col, 1 );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		// TBD
		Sampler[0] = (smpNormalAO);
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;
		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
	}
}
