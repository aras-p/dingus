#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"
#include "lib/shadowmap.fx"


texture tEnv;
samplerCUBE smpEnv = sampler_state {
	Texture = (tEnv);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
		MinFilter = Point; MagFilter = Point; MipFilter = None;
	AddressU = Clamp; AddressV = Clamp;
};


// environment map
float4 cAr;
float4 cAg;
float4 cAb;
float4 cBr;
float4 cBg;
float4 cBb;
float4 cC;

float3 evalSHEnv( float4 n )
{
	float3 x1, x2, x3;

	// Linear + constant polynomial terms
	x1.r = dot(cAr,n);
	x1.g = dot(cAg,n);
	x1.b = dot(cAb,n);

	// 4 of the quadratic polynomials
	float4 vB = n.xyzz * n.yzzx;
	x2.r = dot(cBr,vB);
	x2.g = dot(cBg,vB);
	x2.b = dot(cBb,vB);

	// Final quadratic polynomial
	float vC = n.x*n.x - n.y*n.y;
	x3 = cC.rgb * vC;

	return x1 + x2 + x3;
}

// --------------------------------------------------------------------------
//  vertex shader

struct SInput {
	float4	pos	: POSITION;
	float4	nao	: NORMAL;
};

struct SOutput {
	float4	pos	: POSITION;
	half4	diffao : COLOR0;
	half3	n	: TEXCOORD0;
	half3	v	: TEXCOORD1;
	float3	shz	: TEXCOORD2;
};

SOutput vsMain( SInput i ) {
	SOutput o;
	o.pos	= mul( i.pos, mViewProj );
	float3 n = i.nao.xyz*2-1;
	o.diffao.rgb = evalSHEnv( float4(n,1) );
	o.diffao.a = i.nao.w * 0.9 + 0.1;

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
	color += (i.diffao.rgb * MATERIAL_DIFFUSE + spec) * i.diffao.a;

	// lighting: sunlight
	half shadow = gSampleShadow( smpShadow, i.shz.xy, i.shz.z );
	color += saturate(dot(i.n,-vLightDir)) * fLightIntensity * shadow;

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
