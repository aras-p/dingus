#include "lib/shared.fx"
#include "lib/structs.fx"

SPos vsMain( SPos i ) {
	SPos o;
	o.pos = mul( i.pos, mViewProj );
	return o;
}

half4 psMain( SPos i ) : COLOR {
	return 0.6;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
		FVF = Xyz;
	}
	pass PLast {
	}
}

technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = NULL;

		TextureFactor = 0xA0A0A0A0;
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = TFactor;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = TFactor;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
	}
}
