#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

texture		tRefl;
sampler2D	smpRefl = sampler_state {
	Texture = (tRefl);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


SPosColTexp3 vsMain( SPosN i ) {
	SPosColTexp3 o;
	o.pos = mul( i.pos, mViewProj );
	o.uvp[0] = mul( i.pos, mShadowProj );
	o.uvp[1] = mul( i.pos, mShadowProj2 );
	o.uvp[2] = mul( i.pos, mViewTexProj );
	o.color = gWallLight( i.pos.xyz, i.normal*2-1 );
	return o;
}

half4 psMain( SPosColTexp3 i ) : COLOR {
	half sh1 = tex2Dproj( smpShadow, i.uvp[0] ).r;
	half sh2 = tex2Dproj( smpShadow2, i.uvp[1] ).r;
	half shadow = min( sh1, sh2 );
	half3 col = i.color * shadow;
	col += tex2Dproj( smpRefl, i.uvp[2] ) * 0.15;
	return half4( col, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();

		//FillMode = Wireframe;
	}
	pass PLast {
		FillMode = Solid;
		Texture[0] = NULL;
	}
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		FillMode = Wireframe;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
		FillMode = Solid;
	}
}
