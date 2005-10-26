#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"

float3	vPos;

texture tEnv;
samplerCUBE smpEnv = sampler_state {
	Texture = (tEnv);
	MagFilter = Point; MinFilter = Point; MipFilter = Point;
};


// --------------------------------------------------------------------------
//  vertex shader

struct SInput {
	float4	pos	: POSITION;
};

struct SOutput {
	float4	pos	: POSITION;
	half3	uvw	: TEXCOORD0;
};

SOutput vsMain( SInput i ) {
	SOutput o;
	o.uvw = i.pos.xyz;
	i.pos.xyz *= 500000;
	i.pos.xyz += vPos;
	o.pos = mul( i.pos, mViewProj );
	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	float3 c = texCUBE( smpEnv, i.uvw );
	return EncodeRGBE8( c );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		ZWriteEnable = False;
		ZEnable = False;
	}
	RESTORE_PASS
}
