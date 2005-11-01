#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"
#include "lib/shadowmap.fx"


texture tEnv;
samplerCUBE smpEnv = sampler_state {
	Texture = (tEnv);
	MagFilter = Point; MinFilter = Point; MipFilter = Point;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
#ifdef DST_SHADOWS
	MinFilter = Linear; MagFilter = Linear; MipFilter = None;
#else
	MinFilter = Point; MagFilter = Point; MipFilter = None;
#endif
	AddressU = Clamp; AddressV = Clamp;
};


// --------------------------------------------------------------------------
//  vertex shader

struct SInput {
	float4	pos	: POSITION;
	float4	nao	: NORMAL;
};

struct SOutput {
	float4	pos	: POSITION;
	half3	diff : COLOR0;
	half2	ao	: COLOR1;
	half3	n	: TEXCOORD0;
	half3	v	: TEXCOORD1;
	float3	shz	: TEXCOORD2;
};

SOutput vsMain( SInput i ) {
	SOutput o;
	o.pos	= mul( i.pos, mViewProj );
	float3 n = i.nao.xyz*2-1;
	o.diff = evalSHEnv( float4(n,1) );
	o.ao.xy = float2( i.nao.w * 0.9 + 0.1, i.nao.w * 0.6 + 0.4 );

	o.n = n;

	// view vector
	o.v = i.pos.xyz - vEye;

	gShadowProj( i.pos, mShadowProj, mLightViewProj, o.shz.xy, o.shz.z );

	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	// sample specular envmap
	/*
	const float REFLECTIVITY = 0.01;
	i.n = normalize(i.n);
	half3 refl = reflect( i.v, i.n );
	half3 spec = texCUBE( smpEnv, refl ).rgb * REFLECTIVITY;
	*/
	half3 spec = 0;

	const float MATERIAL_DIFFUSE = 0.7;

	// lighting: environment
	half3 color = 0;
	color += (i.diff * MATERIAL_DIFFUSE + spec) * i.ao.x;

	// lighting: sunlight
	half shadow = gSampleShadow( smpShadow, i.shz.xy, i.shz.z );
	color += saturate(dot(i.n,-vLightDir)) * fLightIntensity * shadow * i.ao.y;

	return EncodeRGBE8( color );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
