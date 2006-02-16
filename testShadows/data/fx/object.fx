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
float3	vInvShadowSize;


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


#if D_SHD_FILTER == 3
half4 psMain( SOutput i, float2 pos : VPOS ) : COLOR
#else
half4 psMain( SOutput i ) : COLOR
#endif
{
	// shadow
#if D_SHD_FILTER == 3
	half shadow = gSampleShadow( smpShadow, i.shz, vInvShadowSize.xy, pos.xy );
#else
	half shadow = gSampleShadow( smpShadow, i.shz, vInvShadowSize.xy );
#endif
	
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
		#if D_SHD_FILTER == 3
		VertexShader = compile vs_3_0 vsMain();
		PixelShader = compile ps_3_0 psMain();
		#else
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		#endif
	}
	RESTORE_PASS
}
