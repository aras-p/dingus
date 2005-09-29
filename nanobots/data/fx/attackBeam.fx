#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float3	vOrigin;
float3	vLength;
float3	vDirection;
float	fAlpha;

float4	vColor;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

// --------------------------------------------------------------------------

// Positions contains velocity
// Normal contains:
//	x,y - corner offset
//	z	- time alpha offset (0..1)

SPosColTex vsMainFF( SPosN i ) {
	SPosColTex o;

	float3 pos = vOrigin + i.normal.z * vLength + fAlpha*i.pos.xyz;
	float size = 0.2 + (1-i.normal.z) * 0.3;
	pos.xyz += i.normal.x * vCameraX * 0.4;
	pos.xyz += i.normal.y * vCameraY * 0.4;
	o.pos = mul( float4(pos,1), mViewProj );
	o.color.rgb = vColor.rgb; //float3(0.3,0.8,1.0);
	
	float alpha = fAlpha;
	o.color.a = (alpha<0.3) ? (alpha/0.3) : (1-(alpha-0.3)/0.7);
	o.color.a *= 0.75;
	o.color.a *= saturate( gFog( float4(pos,1) ) );

	o.uv = i.normal.xy+0.5;
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
