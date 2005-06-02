#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float3	vOrigin;
float	fAlpha;

float4	vColor;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

// --------------------------------------------------------------------------

SPosColTex vsMainFF( SPosNTex i ) {
	SPosColTex o;
	i.pos.xyz *= float3(4,2,4);
	i.pos.xyz += vOrigin;
	o.pos = mul( i.pos, mViewProj );

	float3 vn = mul( i.normal, (float3x3)mView );
	float rim = saturate(0.8-abs(vn.z));

	float a = saturate( min( fAlpha, 1-fAlpha ) );

	o.color = float4(vColor.rgb,a*rim);
	o.color.a *= saturate( gFog( i.pos ) );

	o.uv = i.uv + fAlpha*0.1;
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
