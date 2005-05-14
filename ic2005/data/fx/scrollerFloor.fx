#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"


SPosTexp vsMain( SPosN i ) {
	SPosTexp o;
	i.pos.xyz = i.pos.xzy * 4;
	o.pos = mul( i.pos, mViewProj );
	o.uvp = mul( i.pos, mShadowProj );
	return o;
}

half4 psMain( SPosTexp i ) : COLOR {
	half3 col = tex2Dproj( smpShadow, i.uvp );
	return half4( col, 1 );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		CullMode = None;
	}
	pass PLast {
		Texture[0] = NULL;
		CullMode = <iCull>;
	}
}
