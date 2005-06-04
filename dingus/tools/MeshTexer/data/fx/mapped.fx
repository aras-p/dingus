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

texture		tGloss;
sampler2D	smpGloss = sampler_state {
	Texture = (tGloss);
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
	
	float3 tolight = vLightPos - wpos;
	tolight = normalize( tolight );
	tolight = mul( tolight, wT );
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

	// construct gloss map
	spec *= gMapGloss( i.opos, smpGloss, scales );

	half3 col = cDiff * diffuse + spec + amb;

	return half4( col, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
	}
}
