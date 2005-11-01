#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"
#include "lib/shadowmap.fx"


float3	vPos;
float	fSize;


texture		tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
#ifdef DST_SHADOWS
	MinFilter = Linear; MagFilter = Linear; MipFilter = None;
#else
	MinFilter = Point; MagFilter = Point; MipFilter = None;
#endif
	AddressU = Clamp; AddressV = Clamp;
};


// --------------------------------------------------------------------------
//  vertex shader

struct SInput {
	float4	pos	: POSITION;
	float3	n	: NORMAL;
};

struct SOutput {
	float4	pos	: POSITION;
	half3	diff : COLOR0;
	float3	shz	: TEXCOORD0;
};

SOutput vsMain( SInput i ) {
	SOutput o;
	i.pos.xyz = i.pos.xzy;
	i.pos.xyz *= fSize * 5;
	i.pos.xz += vPos.xz;
	o.pos	= mul( i.pos, mViewProj );

	o.diff = evalSHEnv( float4(0,1,0,1) );

	gShadowProj( i.pos, mShadowProj, mLightViewProj, o.shz.xy, o.shz.z );

	return o;
}


half4 psMain( SOutput i ) : COLOR
{
	const float MATERIAL_DIFFUSE = 0.7;

	// lighting: environment
	half3 color = 0;
	color += (i.diff * MATERIAL_DIFFUSE);

	// lighting: sunlight
	half shadow = gSampleShadow( smpShadow, i.shz.xy, i.shz.z );
	color += saturate(dot(half3(0,1,0),-vLightDir)) * fLightIntensity * shadow;

	return EncodeRGBE8( color );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	RESTORE_PASS
}
