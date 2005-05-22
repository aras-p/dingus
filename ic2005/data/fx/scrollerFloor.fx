#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"


SPosTexTexp vsMain( SPosNTex i ) {
	SPosTexTexp o;
	i.pos.xyz = i.pos.xzy;
	i.pos.x *= 8;
	i.pos.z *= 15;
	i.pos.x += 1;
	i.pos.z += 1;
	o.pos = mul( i.pos, mViewProj );
	o.uv = i.uv;
	o.uvp = mul( i.pos, mShadowProj );
	return o;
}

SPosCol vsMainFFP( SPosNTex i ) {
	SPosCol o;
	i.pos.xyz = i.pos.xzy;
	i.pos.x *= 8;
	i.pos.z *= 15;
	i.pos.x += 1;
	i.pos.z += 1;
	o.pos = mul( i.pos, mViewProj );
	float col = lerp( 1.1, 224.0/255.0, i.uv.x );
	o.color = col;
	return o;
}

half4 psMain( SPosTexTexp i ) : COLOR {
	half col = lerp( 1.1, 224.0/255.0, i.uv.x );
	col *= tex2Dproj( smpShadow, i.uvp ).r;
	return half4( col, col, col, 0 );
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

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFFP();
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
