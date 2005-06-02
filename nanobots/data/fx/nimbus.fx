#include "lib/shared.fx"
#include "lib/structs.fx"

float3	vOrigin;
float	fScale;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosNTex i ) {
	SPosColTex o;
	i.pos.xyz *= fScale;
	i.pos.xyz += vOrigin;
	o.pos = mul( i.pos, mViewProj );

	float3 vn = mul( i.normal, (float3x3)mView );
	float rim = abs(vn.z) + 0.3;

	o.color.rgb = 1;
	o.color.a = rim;
	i.uv.x = i.uv.x * 7 + fTime*0.6;
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
		ZWriteEnable = False;

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
		AlphaBlendEnable = False;
		ZWriteEnable = True;

		CullMode = <iCull>;
	}
}
