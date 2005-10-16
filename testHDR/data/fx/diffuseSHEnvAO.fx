#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWVP;


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


struct SInput {
	float4	pos	: POSITION;
	float4	nao	: NORMAL;
};

SPosCol vsMain( SInput i ) {
	SPosCol o;
	o.pos	= mul( i.pos, mWVP );
	o.color.rgb = evalSHEnv( float4(i.nao.xyz*2-1,1) );
	o.color.a = i.nao.w;
	return o;
}


half4 psMain( SPosCol i ) : COLOR
{
	return i.color * i.color.a;
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
