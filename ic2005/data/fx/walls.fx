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
	#define WALL_OUTPUT SPosColTexp3
	#define WALL_SHCRD uvp[0]
	#define WALL_S2CRD uvp[1]
	#define WALL_RFCRD uvp[2]
#elif defined(WALL_SHADOW) && defined(WALL_REFL)
	#define WALL_OUTPUT SPosColTexp2
	#define WALL_SHCRD uvp[0]
	#define WALL_RFCRD uvp[1]
#elif defined(WALL_SHADOW) || defined(WALL_REFL)
	#define WALL_OUTPUT SPosColTexp
	#define WALL_SHCRD uvp
	#define WALL_RFCRD uvp
#else
	#define WALL_OUTPUT SPosCol
#endif



#ifdef WALL_REFL
	texture		tRefl;
	sampler2D	smpRefl = sampler_state {
		Texture = (tRefl);
		MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
		AddressU = Clamp; AddressV = Clamp;
	};
#endif



WALL_OUTPUT vsMain( WALL_INPUT i ) {
	WALL_OUTPUT o;
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
	o.color = gWallLight( i.pos.xyz, WALL_N );
	return o;
}


half4 psMain( WALL_OUTPUT i ) : COLOR {
	half4 col = i.color;
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
