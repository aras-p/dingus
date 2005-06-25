#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWorldView;
float4x4	mWVP;


texture		tBaked;
sampler2D	smpBaked = sampler_state {
	Texture = (tBaked);
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
	float3 halfang	: TEXCOORD1;	// object space
};


SOutput vsMain( SPosTex i ) {
	SOutput o;

	float3 wpos = mul( i.pos, mWorld );
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

	// sample diffuse/gloss map
	half4 cBase = tex2D( smpBaked, i.uv );
	half3 cDiff = cBase.rgb;
	spec *= cBase.a;

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