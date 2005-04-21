#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/shadowmap.fx"

texture		tNormal;
sampler2D	smpNormal = sampler_state {
	Texture = (tNormal);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tShadow1;
sampler2D	smpShadow1 = sampler_state {
	Texture = (tShadow1);
	MinFilter = Point; MagFilter = Point; MipFilter = None;
	AddressU = Clamp; AddressV = Clamp;
};
texture		tShadow2;
sampler2D	smpShadow2 = sampler_state {
	Texture = (tShadow2);
	MinFilter = Point; MagFilter = Point; MipFilter = None;
	AddressU = Clamp; AddressV = Clamp;
};
texture		tShadow3;
sampler2D	smpShadow3 = sampler_state {
	Texture = (tShadow3);
	MinFilter = Point; MagFilter = Point; MipFilter = None;
	AddressU = Clamp; AddressV = Clamp;
};
texture		tShadow4;
sampler2D	smpShadow4 = sampler_state {
	Texture = (tShadow4);
	MinFilter = Point; MagFilter = Point; MipFilter = None;
	AddressU = Clamp; AddressV = Clamp;
};


int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float3 uvz0		: TEXCOORD1;
	float3 uvz1		: TEXCOORD2;
	float3 uvz2		: TEXCOORD3;
	float3 uvz3		: TEXCOORD4;
	float3x3 skin	: TEXCOORD5;
};

SOutput vsMain0( SInput0 i ) {
	SOutput o;
	
	// skin pos/n

	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin );
	o.skin = (float3x3)skin;

	// shadow projections
	gShadowProj( o.pos, mShadowProj0, mLightViewProj0, o.uvz0.xy, o.uvz0.z );
	gShadowProj( o.pos, mShadowProj1, mLightViewProj1, o.uvz1.xy, o.uvz1.z );
	gShadowProj( o.pos, mShadowProj2, mLightViewProj2, o.uvz2.xy, o.uvz2.z );
	gShadowProj( o.pos, mShadowProj3, mLightViewProj3, o.uvz3.xy, o.uvz3.z );

	o.pos = mul( o.pos, mViewProj );

	o.uv.x = i.uv.x;
	o.uv.y = -i.uv.y;

	return o;
}

float4 psMain( SOutput i ) : COLOR {
	// sample normal map, compute lightings
	half3 normal0 = tex2D( smpNormal, i.uv ).rgb*2-1;
	half3 normal;
	normal.x = normal0.x;
	normal.y = normal0.z;
	normal.z = normal0.y;

	// transform into world space
	normal = mul( normal, i.skin );

	half4 colors;
	colors.x = saturate( dot( normal, vLightDir0 ) );
	colors.y = saturate( dot( normal, vLightDir1 ) );
	colors.z = saturate( dot( normal, vLightDir2 ) );
	colors.w = saturate( dot( normal, vLightDir3 ) );
	colors *= 1.0 / (128.0*0.5);

	// sample shadow maps
	half4 shadows;
	shadows.x = gSampleShadow( smpShadow1, i.uvz0.xy, i.uvz0.z );
	shadows.y = gSampleShadow( smpShadow2, i.uvz1.xy, i.uvz1.z );
	shadows.z = gSampleShadow( smpShadow3, i.uvz2.xy, i.uvz2.z );
	shadows.w = gSampleShadow( smpShadow4, i.uvz3.xy, i.uvz3.z );
	
	shadows *= colors;
	return dot( shadows, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psMain();

		AlphaBlendEnable = True;
		SrcBlend = One;
		DestBlend = One;

		ZWriteEnable = False;
	}
	pass PLast {
		Texture[0] = NULL;
		Texture[1] = NULL;
		Texture[2] = NULL;
		Texture[3] = NULL;

		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
