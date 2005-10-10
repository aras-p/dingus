#include "lib/shared.fx"
#include "lib/structs.fx"

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosColTex i ) {
	SPosColTex o;
	o.pos = mul( i.pos, mViewProj );
	o.color = i.color;
	o.uv = i.uv;
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
		CullMode = None;
		ZWriteEnable = False;

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
