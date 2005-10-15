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

SPosColTex vsMainFF( SPosNTex i ) {
	SPosColTex o;
	o.pos = mul( i.pos, mWVP );
	float3 wpos = mul( i.pos, mWorld );
	float3 vn = mul( i.normal*2-1, (float3x3)mWorldView );
	float rim = saturate( abs(vn.z) * 1.2 - 0.2 );
	float a = lerp( vColor.a, 0.35, i.pos.y*0.15 );
	o.color.rgb = vColor.rgb;
	o.color.a = a * rim;
	o.color.a *= saturate( min( i.pos.y, 15-i.pos.y ) );
	o.color.a *= saturate( gFogWV( i.pos, mWorldView ) );

	o.uv.x = i.uv.x + fTime * 0.08;
	o.uv.y = i.uv.y - fTime * 0.05;
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
	RESTORE_PASS
}
