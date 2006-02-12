#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/shadowmap.fx"

float4x4 mWorld;
float4x4 mWVP;

float3	vLightPos;
float3	vLightDir;
float	fLightCosAngle;
float4x4	mShadowProj;
float4x4	mLightViewProj;


struct SOutput {
	float4	pos	: POSITION;
	half3	n	: TEXCOORD0;
	half3	tol	: TEXCOORD1;
	float4	shz	: TEXCOORD2;
};

SOutput vsMain( SPosN i )
{
	SOutput o;
	o.pos = mul( i.pos, mWVP );
	
	float3 wpos = mul( i.pos, mWorld );
	
	float3 n = i.normal*2-1;
	o.n = mul( n, (float3x3)mWorld );
	
	o.tol = normalize( vLightPos - wpos );

	gShadowProj( float4(wpos,1), mShadowProj, mLightViewProj, o.shz );

	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	half shadow = gSampleShadow( smpShadow, i.shz );
	
	// spot light
	if( dot( -normalize(i.tol), vLightDir ) < fLightCosAngle )
		shadow = 0;

	// TBD: attenuation
	
	//half shadow = 1;
	const float DIFFUSE = 0.7;
	half color = saturate(dot(i.n,i.tol)) * shadow * DIFFUSE;
	return color;
	
	//return half4(i.n*0.5+0.5,1);
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
