#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

// Normal contains:
//	x,y - corner offset
//	z	- time alpha offset (0..1)
// Color.a contains relative size

SPosColTex vsMainFF( SPosNCol i ) {
	SPosColTex o;
	float alpha = frac( fTime/3.0 + i.normal.z );
	i.pos.y += alpha * 0.3;
	i.pos.xyz += vCameraX * (0.4 * i.color.a * i.normal.x);
	i.pos.xyz += vCameraY * (0.4 * i.color.a * i.normal.y);
	o.pos = mul( i.pos, mViewProj );
	o.color.rgb = i.color.rgb;
	o.color.a = (alpha<0.3) ? (alpha/0.3) : (1-(alpha-0.3)/0.7);
	o.color.a *= 0.3;
	o.color.a *= saturate( gFog( i.pos ) );

	o.uv = i.normal.xy+0.5;
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		ZWriteEnable = False;
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = One;

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
	}
}
