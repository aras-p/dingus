#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float4x4	mWorld;
float4x4	mWorldView;
float4x4	mWVP;

float4		vColor;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


static inline float4 lighting( float3 n ) {
	float4 color = float4(0.4,0.4,0.4,0);
	color += saturate( dot( n, -normalize(float3(0.2,-1,0.2)) ) ) * float4(0.2,0.0,0.0,1.0);
	float3 wn = mul( n, (float3x3)mView );
	color += saturate( dot( wn, -normalize(float3(0.2,-0.2,1)) ) ) * 0.7;
	return color;
}

// --------------------------------------------------------------------------

SPosColTexFog vsMainFF( SPosNTex i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mWVP );
	o.color = lighting( mul( i.normal, (float3x3)mWorld ) ) * vColor;
	o.uv = i.uv;

	o.fog = gFogWV( i.pos, mWorldView );

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
