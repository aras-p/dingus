#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

float3 vLightPos;


SPosColTexp vsMain( SPosN i ) {
	SPosColTexp o;
	o.pos = mul( i.pos, mViewProj );
	o.uvp = mul( i.pos, mShadowProj );
	o.color = gWallLight( i.pos.xyz, i.normal*2-1, vLightPos );
	return o;
}

half4 psMain( SPosColTexp i ) : COLOR {
	half3 col = tex2Dproj( smpShadow, i.uvp ) * i.color;
	return half4( col, 1 );
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
		Texture[0] = NULL;
	}
}
