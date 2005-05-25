#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"
#include "lib/dof.fx"


float3		vLightPos;

#define WALL_DOF

#ifndef WALL_DIFF
#define WALL_DIFF	DEF_WALL_DIFF
#endif
#ifndef WALL_AMB
#define WALL_AMB	DEF_WALL_AMB
#endif


#ifdef WALL_NCOL
	#define WALL_INPUT SPosCol
	#define WALL_N (i.color.xyz)
	#define WALL_A (i.color.w)
#else
	#define WALL_INPUT SPosN
	#define WALL_N (i.normal)
	#define WALL_A 1.0
#endif


#if defined(WALL_SH2REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half4  na 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp[3]	: TEXCOORD0;
	};
	#define WALL_SHCRD uvp[0]
	#define WALL_S2CRD uvp[1]
	#define WALL_RFCRD uvp[2]
	#define WALL_DOFCRD uvp[0].z
#elif defined(WALL_SHADOW) && defined(WALL_REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half4  na 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp[2]	: TEXCOORD0;
	};
	#define WALL_SHCRD uvp[0]
	#define WALL_RFCRD uvp[1]
	#define WALL_DOFCRD uvp[0].z
#elif defined(WALL_SHADOW) || defined(WALL_REFL)
	struct SOutput {
		float4 pos		: POSITION;
		half4  na 		: COLOR0;
		half3  tol 		: COLOR1;
		float4 uvp		: TEXCOORD0;
	};
	#define WALL_SHCRD uvp
	#define WALL_RFCRD uvp
	#define WALL_DOFCRD uvp.z
#else
	struct SOutput {
		float4 pos		: POSITION;
		half4  na 		: COLOR0;
		half3  tol 		: COLOR1;
	#ifdef WALL_DOF
		float  z		: TEXCOORD0;
	#endif
	};
	#define WALL_DOFCRD z
#endif


#ifdef WALL_SHADOW
	texture	tShadow;
	sampler2D	smpShadow = sampler_state {
		Texture = (tShadow);
		MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
		AddressU = Clamp; AddressV = Clamp;
	};
#endif

#ifdef WALL_SH2REFL
	texture	tShadow2;
	sampler2D	smpShadow2 = sampler_state {
		Texture = (tShadow2);
		MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
		AddressU = Clamp; AddressV = Clamp;
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

	// compute texcoords for projection in pixel shader
#ifdef WALL_SHADOW
	o.WALL_SHCRD = mul( i.pos, mShadowProj );
#endif
#ifdef WALL_SH2REFL
	o.WALL_S2CRD = mul( i.pos, mShadowProj2 );
#endif
#ifdef WALL_REFL
	o.WALL_RFCRD = mul( i.pos, mViewTexProj );
#endif

	// DOF
#ifdef WALL_DOF
	o.WALL_DOFCRD = gCameraDepth( i.pos.xyz );
#endif

	// pass lighting to pixel shader
	o.na.xyz = WALL_N;
	o.na.w = WALL_A;

	float3 tolight = normalize( vLightPos - i.pos.xyz );
	o.tol = tolight*0.5+0.5;

	return o;
}


half4 psMain( SOutput i ) : COLOR {
	// lighting
	half3 n = i.na.xyz*2-1;
	half3 tol = normalize( i.tol*2-1 );
	half l = gWallLightPS( n, tol, WALL_DIFF, WALL_AMB );
	half4 col = l;

	// want more colours - uncomment this!
#if D_FUNKY==1
	col.rgb = cross( n.rgb, tol.rgb )*0.5+0.5;
#endif

#ifdef WALL_SHADOW
	half shadow = tex2Dproj( smpShadow, i.WALL_SHCRD ).r;
	#ifdef WALL_SH2REFL
		half shadow2 = tex2Dproj( smpShadow2, i.WALL_S2CRD ).r;
		shadow = min( shadow, shadow2 );
	#endif
	col.xyz *= shadow;
#endif
#ifdef WALL_REFL
	col.xyz += tex2Dproj( smpRefl, i.WALL_RFCRD ) * 0.25;
#endif

	half4 color = col;

#ifdef WALL_ALPHA
	color.a = i.na.w;
#else
	#ifdef WALL_DOF
		color.a = gBluriness( i.WALL_DOFCRD );
	#endif
#endif

	return color;
}


#ifdef WALL_ALPHA
#ifdef WALL_DOF
half4 psMainA( SOutput i ) : COLOR {
	return half4( 1, 1, 1, gBluriness( i.WALL_DOFCRD ) );
}
#endif
#endif


#ifdef WALL_ALPHA

technique tec20
{
	// blend
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		ColorWriteEnable = Red | Green | Blue;
	}

	// dof alpha
#ifdef WALL_DOF
	pass P1 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMainA();

		AlphaBlendEnable = False;
		ColorWriteEnable = Alpha;
	}
#endif

	pass PLast {
		Texture[0] = NULL;
		ColorWriteEnable = Red | Green | Blue | Alpha;
	}
}

#else

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
	}
}

#endif
