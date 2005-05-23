#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/dof.fx"

float3		vLightPos;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tNormalAO;
sampler2D	smpNormalAO = sampler_state {
	Texture = (tNormalAO);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 tolight	: COLOR0;		// skin space
	float4 toview	: COLOR1;		// skin space
	float2 uv		: TEXCOORD0;
	float  z		: TEXCOORD1;
};

SOutput vsMain0( SInput0 i ) {
	SOutput o;
	
	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin ); // world pos

	o.z = gCameraDepth( o.pos.xyz );

	float3x3 skinT = transpose( (float3x3)skin );

	float3 tolight = normalize( vLightPos - o.pos.xyz );
	tolight = mul( tolight, skinT );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	float3 toview = normalize( vEye - o.pos.xyz );
	toview = mul( toview, skinT );
	o.toview = float4( toview*0.5+0.5, 1 );

	o.pos = mul( o.pos, mViewProj );

	o.uv = i.uv;

	return o;
}

half4 psMain( SOutput i ) : COLOR {
	// sample normal+AO map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;

#if D_AO==1
	half occ = normalAO.a * 0.5 + 0.5;
#else
	half occ = 1;
#endif

#if D_NORMALMAPS==1	
	half diffuse = saturate( dot( normal, i.tolight.xyz*2-1 ) );
	half rim = (1-saturate( dot( normal, i.toview.xyz*2-1 ) ));
	rim *= 0.3;
#else
	half diffuse = 1;
	half rim = 0;
#endif

	half4 timeBlend = tex2D( smpBase, i.uv );
	half lerper = saturate( (fCharTimeBlend-timeBlend.a) / 0.1 );
	half colChar = lerp( 1, timeBlend.g, lerper );

	const half3 cDiff = half3( 1.05, 1.1, 1.2 );

	const half3 cRim = half3( 0.99, 0.98, 0.96 );
	half3 col = cDiff * diffuse + cRim * rim;

	col = col * 0.5 + 0.6 * occ;

	col *= colChar;

	return half4( col, gBluriness(i.z) );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
	}
}



SPosColTex vsMainFFP( SInput0 i ) {
	SPosColTex o;
	
	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin ); // world pos
	float3 wnormal = mul( i.normal*2-1, (float3x3)skin );

	float3 tolight = normalize( vLightPos - o.pos.xyz );
	float diffuse = saturate( dot( wnormal, tolight ) );

	o.pos = mul( o.pos, mViewProj );

	o.color = diffuse * 0.8 + 0.2;

	o.uv = i.uv;

	return o;
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFFP();
		PixelShader = NULL;

		Sampler[0] = (smpNormalAO);
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture | AlphaReplicate;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;

		//FillMode = Wireframe;
	}
	pass PLast {
		//FillMode = Solid;
	}
}
