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



// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosN i ) {
	SPosColTex o;
	i.pos.y += 1;
	o.pos = mul( i.pos, mWVP );
	float3 vn = mul( i.normal*2-1, (float3x3)mWorldView );
	
	o.color = vColor;

	o.uv = vn.xy*0.5+0.4;

	o.color.a *= saturate( gFogWV( i.pos, mWorldView ) );
	return o;
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = One;
		ZWriteEnable = False;
		//CullMode = None;

		Sampler[0] = (smpBase);
		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
		
		// only render where bit 1 is set (inside level)
		StencilEnable = True;
		StencilFunc = Equal;
		StencilRef = 1;
		StencilMask = 1;
		StencilWriteMask = 0;
		StencilFail = Keep;
		StencilPass = Keep;
		StencilZFail = Keep;
	}
	RESTORE_PASS
}
