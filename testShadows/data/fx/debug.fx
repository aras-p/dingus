#include "lib/shared.fx"
#include "lib/structs.fx"

SPosCol vsMain( SPosNCol i ) {
	SPosCol o;
	o.pos = mul( i.pos, mViewProj );
	//float diffuse = saturate( dot( i.normal, -vLightDir ) ) * 0.4 + 0.6;
	float diffuse = 1.0;
	o.color.rgb = i.color.rgb * diffuse;
	o.color.a = i.color.a;
	return o;
}

technique tecFFP
{
	pass PAlpha
	{
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;
		
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		
		CullMode = None;
		ZWriteEnable = False;
		
		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}

	RESTORE_PASS
}
