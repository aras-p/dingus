#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


static inline float lighting( float3 n ) {
	float a = 1-abs(n.y)*0.6;

	float3 vn = mul( n, (float3x3)mView );
	float rim = 1-vn.z;
	rim *= a;

	return rim + 0.2;
}

// --------------------------------------------------------------------------

SPosCol2Tex3F vsMain11( SPosCol i ) {
	SPosCol2Tex3F o;

	float3 n = i.color.xyz*2-1;
	o.color[0] = lighting( n );
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
	float4 cbasex = tex2D( smpBase, i.uv[0] ) * i.color[1].x;
	float4 cbasey = tex2D( smpBase, i.uv[1] ) * i.color[1].y;
	float4 cbasez = tex2D( smpBase, i.uv[2] ) * i.color[1].z;
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
	o.color = lighting( n );

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
	pass PLast {
		FogEnable = False;
	}
}
