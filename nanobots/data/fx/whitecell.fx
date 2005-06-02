#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float4x4	mWorldView;
float4x4	mWVP;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosNTex i ) {
	SPosColTex o;
	o.pos = mul( i.pos, mWVP );
	float3 vn = mul( i.normal, (float3x3)mWorldView );
	float rim = 1 - abs(vn.z);
	o.color = float4(1,1,1,rim*0.75);

	o.color.a *= saturate( gFogWV( i.pos, mWorldView ) );
	o.uv.x = i.uv.x + fTime * 0.08;
	o.uv.y = i.uv.y - fTime * 0.06;
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

		Sampler[0] = <smpBase>;
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
