#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase1;
sampler2D	smpBase1 = sampler_state {
	Texture = (tBase1);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};
texture		tBase2;
sampler2D	smpBase2 = sampler_state {
	Texture = (tBase2);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


static inline half lighting( half3 n, half3 vn ) {
	half a = 1-abs(n.y)*0.6;
	half rim = 1-vn.z;
	rim *= a;
	return rim + 0.2;
}


// --------------------------------------------------------------------------

SPosCol2Tex3F vsMain20( SPosCol i ) {
	SPosCol2Tex3F o;

	float3 n = i.color.xyz*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color[0] = lighting( n, vn );
	o.color[0].w = i.color.w;

	float3 an = abs(n);
	an /= (an.x+an.y+an.z);
	o.color[1] = float4( an, 1 );

	o.uv[0] = float2(i.pos.y,i.pos.z) * 0.087 + fTime*0.0011;
	o.uv[1] = float2(i.pos.x,i.pos.z) * 0.020;
	o.uv[2] = float2(i.pos.x,i.pos.y) * 0.095 + fTime*0.0013;

	i.pos.y += sin( i.pos.x*0.13+i.pos.z*0.17+fTime ) * 0.1;
	o.pos = mul( i.pos, mViewProj );

	o.fog = gFog( i.pos );

	return o;
}

half4 psMain20( SPosCol2Tex3F i ) : COLOR {
	half3 color = i.color[0].xyz;
	half3 an = i.color[1].xyz;
	half alpha = i.color[0].w;

	half4 cbase1x = tex2D( smpBase1, i.uv[0] ) * an.x;
	half4 cbase1y = tex2D( smpBase1, i.uv[1] ) * an.y;
	half4 cbase1z = tex2D( smpBase1, i.uv[2] ) * an.z;
	half4 cbase1 = cbase1x + cbase1y + cbase1z;

	half4 cbase2x = tex2D( smpBase2, i.uv[0] ) * an.x;
	half4 cbase2y = tex2D( smpBase2, i.uv[1] ) * an.y;
	half4 cbase2z = tex2D( smpBase2, i.uv[2] ) * an.z;
	half4 cbase2 = cbase2x + cbase2y + cbase2z;

	half4 cbase = lerp( cbase1, cbase2, alpha );

	cbase.rgb *= color;

	return cbase;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_2_0 vsMain20();
		PixelShader = compile ps_2_0 psMain20();

		FogEnable = True;
	}
	RESTORE_PASS
}


// --------------------------------------------------------------------------

SPosColTex2F vsMain11( SPosCol i ) {
	SPosColTex2F o;

	float3 n = i.color.xyz*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color = lighting( n, vn );
	o.color.w = i.color.w;

	o.uv[0] = o.uv[1] = float2(i.pos.x,i.pos.z) * 0.02;

	i.pos.y += sin( i.pos.x*0.13+i.pos.z*0.17+fTime ) * 0.1;
	o.pos = mul( i.pos, mViewProj );

	o.fog = gFog( i.pos );

	return o;
}

float4 psMain11( SPosColTex2F i ) : COLOR {
	float4 cbase1 = tex2D( smpBase1, i.uv[0] );
	float4 cbase2 = tex2D( smpBase2, i.uv[1] );
	float4 cbase = lerp( cbase1, cbase2, i.color.a );
	cbase.rgb *= i.color.rgb;
	return cbase;
}

technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_1_1 psMain11();

		FogEnable = True;
	}
	RESTORE_PASS
}


// --------------------------------------------------------------------------

SPosColTex2F vsMainFF( SPosCol i ) {
	SPosColTex2F o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color = lighting( n, vn );
	o.color.w = i.color.w;

	o.uv[0] = o.uv[1] = float2(i.pos.x,i.pos.z) * 0.02;

	float d = length(i.pos - vEye);
	o.fog = (vFog.y - d) * vFog.z;

	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		FogEnable = True;

		Sampler[0] = (smpBase1);
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		Sampler[1] = (smpBase2);
		ColorOp[1] = BlendCurrentAlpha;
		ColorArg1[1] = Texture;
		ColorArg2[1] = Current;
		AlphaOp[1] = SelectArg1;
		AlphaArg1[1] = Diffuse;

		ColorOp[2] = Modulate;
		ColorArg1[2] = Current;
		ColorArg2[2] = Diffuse;
		AlphaOp[2] = SelectArg1;
		AlphaArg1[2] = Current;

		ColorOp[3] = Disable;
		AlphaOp[3] = Disable;
	}
	RESTORE_PASS
}
