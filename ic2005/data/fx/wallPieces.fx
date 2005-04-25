#include "lib/shared.fx"
#include "lib/structs.fx"


SPosColTexp vsMain( SPosCol i ) {
	SPosColTexp o;
	float3 tolight = normalize( vLightPos - i.pos.xyz );
	o.pos = mul( i.pos, mViewProj );

	o.uvp = mul( i.pos, mShadowProj );

	float diffuse = max( 0.0, dot( tolight, i.color.xyz*2-1 ) );
	o.color = diffuse * 0.6 + 0.4;
	return o;
}

half4 psMain( SPosColTexp i ) : COLOR {
	half3 col = tex2Dproj( smpShadow, i.uvp ) * i.color;
	return half4( col, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		FVF = Xyz | Diffuse;

		//FillMode = Wireframe;
	}
	pass PLast {
		//FillMode = Solid;
		Texture[0] = NULL;
	}
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;
		FVF = Xyz | Diffuse;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;

		CullMode = None;
	}
	pass PLast {
		CullMode = <iCull>;
	}
}
