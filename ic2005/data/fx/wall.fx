#include "lib/shared.fx"
#include "lib/structs.fx"


texture		tRefl;
sampler2D	smpRefl = sampler_state {
	Texture = (tRefl);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


SPosColTexp2 vsMain( SPosN i ) {
	SPosColTexp2 o;
	float3 tolight = normalize( vLightPos - i.pos.xyz );
	o.pos = mul( i.pos, mViewProj );

	o.uvp[0] = mul( i.pos, mShadowProj );
	o.uvp[1] = mul( i.pos, mViewTexProj );

	float diffuse = max( 0.0, dot( tolight, i.normal ) );
	o.color = diffuse * 0.6 + 0.4;
	return o;
}

half4 psMain( SPosColTexp2 i ) : COLOR {
	half3 col = tex2Dproj( smpShadow, i.uvp[0] ) * i.color;
	col += tex2Dproj( smpRefl, i.uvp[1] ) * 0.2;
	return half4( col, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		FVF = Xyz | Normal | Diffuse;

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
		FVF = Xyz | Normal | Diffuse;

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
