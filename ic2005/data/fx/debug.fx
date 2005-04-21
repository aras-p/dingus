#include "lib/shared.fx"
#include "lib/structs.fx"

SPosCol vsMain( SPosNCol i ) {
	SPosCol o;
	o.pos = mul( i.pos, mViewProj );
	const float3 lightDir = -normalize( float3( 0.2, -1.0, 0.5 ) );
	//float diffuse = saturate( dot( i.normal, lightDir ) ) * 0.6 + 0.4;
	float diffuse = 1.0;
	//o.color.rgb = i.color.rgb * diffuse;
	//o.color.a = 0.1;
	o.color = i.color;
	return o;
}

technique tecFFP {
	pass POpaque {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;
		FVF = Xyz | Normal | Diffuse;

		AlphaTestEnable = False;

		DepthBias = -0.0001f;
		SlopeScaleDepthBias = -0.01f;

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		ZFunc = LessEqual;
		//ZWriteEnable = False;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;

		CullMode = CCW;
	}

	/*
	pass PAlpha {
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;

		AlphaTestEnable = False;
		ZWriteEnable = False;
	}
	*/
	
	pass PLast {
		DepthBias = 0;
		SlopeScaleDepthBias = 0;

		AlphaBlendEnable = False;
		ZWriteEnable = True;
		CullMode = <iCull>;
	}
}
