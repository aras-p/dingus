#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"
#include "lib/dof.fx"

float3 vLightPos;


texture	tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


SPosColTexp vsMain( SPosN i ) {
	SPosColTexp o;
	o.pos = mul( i.pos, mViewProj );
	o.uvp = mul( i.pos, mShadowProj );
	o.color = gWallLight( i.pos.xyz, i.normal*2-1, vLightPos );
	o.uvp.z = gCameraDepth( i.pos.xyz );
	return o;
}

half4 psMain( SPosColTexp i ) : COLOR {
#if D_SHADOWS==1
	half3 col = tex2Dproj( smpShadow, i.uvp ) * i.color;
#else
	half3 col = i.color.rgb;
#endif
	return half4( col, gBluriness(i.uvp.z) );
}


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
