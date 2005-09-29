#include "lib/shared.fx"
#include "lib/structs.fx"

float4		vSize;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

SPosTex vsMainFF( SPosTex i ) {
	SPosTex o;
	float4 pos;
	pos.x = i.pos.x * vSize.x + vSize.z;
	pos.z = i.pos.y * vSize.y + vSize.w;
	pos.y = 0.0f;
	pos.w = 1.0f;
	o.pos = mul( pos, mViewProj );
	o.uv = i.uv;
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		CullMode = None;

		AlphaTestEnable = True;
		AlphaFunc = Greater;
		AlphaRef = 50;
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		ZEnable = False;
		ZWriteEnable = False;

		Sampler[0] = (smpBase);

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	RESTORE_PASS
}
