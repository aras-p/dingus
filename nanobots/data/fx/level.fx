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
texture		tNormal;
sampler2D	smpNormal = sampler_state {
	Texture = (tNormal);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


static inline half lighting( half3 n, half3 vn ) {
	half a = 1-abs(n.y)*0.6;

	half rim = 1-vn.z;
	rim *= a;

	return rim + 0.2;
}

// --------------------------------------------------------------------------

struct SOutput20 {
	float4 pos		: POSITION;
	float2 uv[3]	: TEXCOORD0;
	half4  n		: TEXCOORD3; // normal + data
	half3  toview	: TEXCOORD4;
	float  fog		: FOG;
};

SOutput20 vsMain20( SPosCol i ) {
	SOutput20 o;

	o.n.xyz = i.color.xyz*2-1;
	o.n.w = i.color.w;

	o.uv[0] = float2(i.pos.y,i.pos.z) * 0.087 + fTime*0.0011;
	o.uv[1] = float2(i.pos.x,i.pos.z) * 0.020;
	o.uv[2] = float2(i.pos.x,i.pos.y) * 0.095 + fTime*0.0013;

	i.pos.y += sin( i.pos.x*0.13+i.pos.z*0.17+fTime ) * 0.1;
	o.toview = normalize( vEye - i.pos.xyz );

	o.pos = mul( i.pos, mViewProj );

	o.fog = gFog( i.pos );

	return o;
}

half4 psMain20( SOutput20 i ) : COLOR {
	half3 n = i.n.xyz;
	half alpha = i.n.w;

	half3 an = abs(n);
	an /= (an.x+an.y+an.z);

	half4 cbase1x = tex2D( smpBase1, i.uv[0] ) * an.x;
	half4 cbase1y = tex2D( smpBase1, i.uv[1] ) * an.y;
	half4 cbase1z = tex2D( smpBase1, i.uv[2] ) * an.z;
	half4 cbase1 = cbase1x + cbase1y + cbase1z;

	half4 cbase2x = tex2D( smpBase2, i.uv[0] ) * an.x;
	half4 cbase2y = tex2D( smpBase2, i.uv[1] ) * an.y;
	half4 cbase2z = tex2D( smpBase2, i.uv[2] ) * an.z;
	half4 cbase2 = cbase2x + cbase2y + cbase2z;

	half4 cbase = lerp( cbase1, cbase2, alpha );

	half2 cnx = tex2D( smpNormal, i.uv[0]*2 )*2-1;
	half2 cny = tex2D( smpNormal, i.uv[1]*4 )*2-1;
	half2 cnz = tex2D( smpNormal, i.uv[2]*2 )*2-1;

	const half STRENGTH = 0.3;
	n.yz += cnx * an.x * STRENGTH;
	n.xz += cny * an.y * STRENGTH;
	n.xy += cnz * an.z * STRENGTH;
	half3 vn = mul( n, (float3x3)mView );
	half color = lighting( n, vn );

	cbase *= color;

	return cbase;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_2_0 vsMain20();
		PixelShader = compile ps_2_0 psMain20();

		FogEnable = True;
	}
	pass PLast {
		FogEnable = False;
	}
}



// --------------------------------------------------------------------------

SPosCol2Tex3F vsMain11( SPosCol i ) {
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

float4 psMain11( SPosCol2Tex3 i ) : COLOR {
	float4 cbasex = tex2D( smpBase1, i.uv[0] ) * i.color[1].x;
	float4 cbasey = tex2D( smpBase1, i.uv[1] ) * i.color[1].y;
	float4 cbasez = tex2D( smpBase1, i.uv[2] ) * i.color[1].z;
	float4 cbase = cbasex + cbasey + cbasez;
	cbase *= i.color[0];
	return cbase;
}

technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_1_1 psMain11();

		FogEnable = True;
	}
	pass PLast {
		FogEnable = False;
	}
}

// --------------------------------------------------------------------------

SPosColTexFog vsMainFF( SPosCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	float3 vn = mul( n, (float3x3)mView );
	o.color = lighting( n, vn );

	o.uv = float2(i.pos.x,i.pos.z) * 0.02;

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

		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
		FogEnable = False;
	}
}
