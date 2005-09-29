#include "lib/shared.fx"
#include "lib/structs.fx"

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
	float4 color;
	color = -n.z;
	return color;
}

// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosNTex i ) {
	SPosColTex o;
	o.pos = mul( i.pos, mWVP );
	o.color = lighting( mul( i.normal, (float3x3)mWorldView ) );
	o.uv = i.uv * 2 + float2(-fTime*0.1,0);
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

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
