#include "lib/shared.fx"
#include "lib/structs.fx"

float3	vOrigin;
float	fAlpha;

texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

// --------------------------------------------------------------------------

// Positions contains position
// Normal contains:
//	x,y - corner offset
//	z	- time alpha offset (0..1)

SPosColTex vsMainFF( SPosN i ) {
	SPosColTex o;

	float a = saturate( (fAlpha - i.normal.z)/(1-i.normal.z) );

	float3 pos = vOrigin + i.pos.xyz;
	pos.y += a;
	float size = 1 - i.normal.z;
	pos.xyz += i.normal.x * vCameraX * size;
	pos.xyz += i.normal.y * vCameraY * size;
	o.pos = mul( float4(pos,1), mViewProj );
	o.color.rgb = 1;

	float2 uv = i.normal.xy+0.5;
	float2 uvo = float2(a*16,a*4);
	uvo -= frac(uvo);
	uv += uvo;

	o.color.a = 1;
	o.color.a = min( o.color.a, a*4 );
	o.color.a = min( o.color.a, (1-a)*2 );
	
	o.uv = uv * 0.25;
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
	pass PLast {
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
