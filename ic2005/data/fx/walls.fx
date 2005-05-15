#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

#ifdef WALL_NCOL
	#define WALL_INPUT SPosCol
	#define WALL_N (i.color.xyz*2-1)
#else
	#define WALL_INPUT SPosN
	#define WALL_N (i.normal*2-1)
#endif


#if defined(WALL_SH2REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half3  n 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp[3]	: TEXCOORD0;
	};
	#define WALL_SHCRD uvp[0]
	#define WALL_S2CRD uvp[1]
	#define WALL_RFCRD uvp[2]
#elif defined(WALL_SHADOW) && defined(WALL_REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half3  n 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp[2]	: TEXCOORD0;
	};
	#define WALL_SHCRD uvp[0]
	#define WALL_RFCRD uvp[1]
#elif defined(WALL_SHADOW) || defined(WALL_REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half3  n 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp		: TEXCOORD0;
	};
	#define WALL_SHCRD uvp
	#define WALL_RFCRD uvp
#else
	struct SOutput {
		float4 pos		: POSITION;
		half3  n 		: COLOR0;
		half3  tol 		: COLOR1;
	};
#endif



#ifdef WALL_REFL
	texture		tRefl;
	sampler2D	smpRefl = sampler_state {
		Texture = (tRefl);
		MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
		AddressU = Clamp; AddressV = Clamp;
	};
#endif



SOutput vsMain( WALL_INPUT i ) {
	SOutput o;
	o.pos = mul( i.pos, mViewProj );
#ifdef WALL_SHADOW
	o.WALL_SHCRD = mul( i.pos, mShadowProj );
#endif
#ifdef WALL_SH2REFL
	o.WALL_S2CRD = mul( i.pos, mShadowProj2 );
#endif
#ifdef WALL_REFL
	o.WALL_RFCRD = mul( i.pos, mViewTexProj );
#endif
	o.n = WALL_N*0.5+0.5;
	float3 tolight = normalize( vLightPos - i.pos.xyz );
	o.tol = tolight*0.5+0.5;
	return o;
}


half4 psMain( SOutput i ) : COLOR {
	// lighting
	half3 n = i.n*2-1;
	half3 tol = normalize( i.tol*2-1 );
	half l = gWallLightPS( n, tol );
	half4 col = l;

#ifdef WALL_SHADOW
	half shadow = tex2Dproj( smpShadow, i.WALL_SHCRD ).r;
	#ifdef WALL_SH2REFL
		half shadow2 = tex2Dproj( smpShadow2, i.WALL_S2CRD ).r;
		shadow = min( shadow, shadow2 );
	#endif
	col.xyz *= shadow;
#endif
#ifdef WALL_REFL
	col.xyz += tex2Dproj( smpRefl, i.WALL_RFCRD ) * 0.15;
#endif
	return col;
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
	}
}