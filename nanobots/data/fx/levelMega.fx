#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

// --------------------------------------------------------------------------

SPosColTexFog vsMainFF( SPosCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	o.color = 1-abs(n.y)*0.6;

	o.uv = float2(i.pos.x,i.pos.z) * 0.02;

	o.fog = gFog( i.pos );

	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		FogEnable = True;

		CullMode = None;

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
		CullMode = <iCull>;
	}
}
