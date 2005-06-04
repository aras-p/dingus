#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWorld;
float4x4	mWorldView;
float4x4	mWVP;


texture		tNormalAO;
sampler2D	smpNormalAO = sampler_state {
	Texture = (tNormalAO);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 tolight	: COLOR0;		// skin space
	float4 toview	: COLOR1;		// skin space
	float2 uv		: TEXCOORD0;
};


SOutput vsMain( SPosTex i ) {
	SOutput o;

	float3 wpos = mul( i.pos, mWorld );
	o.pos = mul( i.pos, mWVP );

	float3x3 wT = transpose( (float3x3)mWorld );
	
	float3 tolight = vLightPos - wpos;
	//tolight.y *= 0.8; // make light more horizontal :)
	tolight = normalize( tolight );
	tolight = mul( tolight, wT );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	float3 toview = normalize( vEye - wpos );
	toview = mul( toview, wT );
	o.toview = float4( toview*0.5+0.5, 1 );

	o.uv = i.uv;

	return o;
}

half4 psMain( SOutput i ) : COLOR {
	// sample normal+AO map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;
	half occ = normalAO.a * 1.0 + 0.0;
	
	half diffuse = saturate( dot( normal, i.tolight.xyz*2-1 ) );
	half rim = (1-saturate( dot( normal, i.toview.xyz*2-1 ) ));
	rim *= 0.3;

	const half3 cDiff = half3( 1.05, 1.1, 1.2 );
	const half3 cRim = half3( 0.99, 0.98, 0.96 );
	half3 col = cDiff * diffuse + cRim * rim;

	col = col * 0.5 + 0.5 * occ;
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
