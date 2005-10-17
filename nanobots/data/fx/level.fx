#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

static inline half lighting( half3 n, half3 vn ) {
	const float3 lightDir = normalize(float3(0.6,-1.0f,0.2));
	half diff = saturate( dot(n,lightDir)*0.3+0.5 );

	half a = 1-abs(n.y)*0.6;
	half rim = 1-vn.z;
	rim *= a;
	return diff + rim;
}


// --------------------------------------------------------------------------

SPosCol2Tex3F vsMain20( SPosNCol i ) {
	SPosCol2Tex3F o;

	float3 n = i.normal*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color[0] = lighting( n, vn ) * i.color;

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
	half3 color = i.color[0].rgb;
	half3 an = i.color[1].xyz;

	half3 cbasex = tex2D( smpBase, i.uv[0] ).rgb * an.x;
	half3 cbasey = tex2D( smpBase, i.uv[1] ).rgb * an.y;
	half3 cbasez = tex2D( smpBase, i.uv[2] ).rgb * an.z;
	half3 cbase = cbasex + cbasey + cbasez;

	cbase *= color;

	return half4( cbase, 1 );
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

SPosColTexFog vsMain11( SPosNCol i ) {
	SPosColTexFog o;

	float3 n = i.normal*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color = lighting( n, vn ) * i.color;

	o.uv = float2(i.pos.x,i.pos.z) * 0.020;

	i.pos.y += sin( i.pos.x*0.13+i.pos.z*0.17+fTime ) * 0.1;
	o.pos = mul( i.pos, mViewProj );

	o.fog = gFog( i.pos );

	return o;
}

float4 psMain11( SPosColTexFog i ) : COLOR {
	float4 cbase = tex2D( smpBase, i.uv );
	return cbase * i.color;
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

SPosColTexFog vsMainFF( SPosNCol i ) {
	SPosColTexFog o;

	float3 n = i.normal*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color = lighting( n, vn ) * i.color;

	o.uv = float2(i.pos.x,i.pos.z) * 0.020;

	o.pos = mul( i.pos, mViewProj );

	o.fog = gFog( i.pos );

	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		FogEnable = True;

		Sampler[0] = (smpBase);
		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	RESTORE_PASS
}
