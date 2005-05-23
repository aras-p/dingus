#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/commonWalls.fx"
#include "lib/dof.fx"

float3 vLightPos;


int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
};

SPosColTexp vsMain0( SInput0 i ) {
	SPosColTexp o;
	
	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin ); // world pos
	float3 n = mul( i.normal*2-1, (float3x3)skin );


	o.uvp = mul( o.pos, mShadowProj );
	o.uvp.z = gCameraDepth( o.pos.xyz );

	o.color = gWallLight( o.pos.xyz, n, vLightPos );
	o.pos = mul( o.pos, mViewProj );

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
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
	}
}
