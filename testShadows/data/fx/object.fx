#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/shadowmap.fx"

float4x4 mWorld;
float4x4 mWVP;
float3	 vColor;

float3	vLightPos;
float3	vLightDir;
float3	vLightColor;
float	fLightCosAngle;
float4x4	mShadowProj;


struct SOutput {
	float4	pos	: POSITION;
	float3	wpos : TEXCOORD0;
	half3	n	: TEXCOORD1;
	float4	shz	: TEXCOORD2;
	half3	col : COLOR0;
};

SOutput vsMain( SPosN i )
{
	SOutput o;
	o.pos = mul( i.pos, mWVP );
	
	float3 wpos = mul( i.pos, mWorld );
	
	float3 n = i.normal*2-1;
	o.n = mul( n, (float3x3)mWorld );
	
	o.wpos = wpos;
	
	o.col = vColor * vLightColor;

	gShadowProj( float4(wpos,1), mShadowProj, o.shz );

	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	// shadow
	half shadow = gSampleShadow( smpShadow, i.shz );
	
	// spot light
	float3 tol = normalize( vLightPos - i.wpos );
	if( dot( -tol, vLightDir ) < fLightCosAngle )
		shadow = 0;

	// TBD: attenuation
	
	half3 color = i.col.rgb * (saturate(dot(i.n,tol)) * shadow);
	return half4( color, 1 );
	
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
