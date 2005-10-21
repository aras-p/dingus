#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"


float4x4	mWVP;

texture tEnv;
samplerCUBE smpEnv = sampler_state {
	Texture = (tEnv);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
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
};

SOutput vsMain( SInput i ) {
	SOutput o;
	o.pos	= mul( i.pos, mWVP );
	float3 n = i.nao.xyz*2-1;
	o.diffao.rgb = evalSHEnv( float4(n,1) );
	o.diffao.a = i.nao.w * 0.7 + 0.4;

	o.n = n;

	// view vector
	o.v = i.pos.xyz - vEye;

	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	// sample specular envmap
	const float REFLECTIVITY = 0.001;
	i.n = normalize(i.n);
	half3 refl = reflect( i.v, i.n );
	half3 spec = texCUBE( smpEnv, refl ).rgb * REFLECTIVITY;

	// diffuse
	half3 diff = i.diffao.rgb;

	// AO
	half ao = i.diffao.a;

	half3 color = (diff + spec) * ao;

	// encode into RGBE8
	return EncodeRGBE8( color );
	//return half4( color, 1 );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
