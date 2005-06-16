#include "lib/shared.fx"
#include "lib/structs.fx"

SPosCol vsMain11( SPos i ) {
	SPosCol o;
	o.pos = mul( i.pos, mViewProj );
	o.color = 0.4;
	return o;
}


technique tecFFP {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = NULL;
		CullMode = None;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	pass PLast {
		CullMode = <iCull>;
	}
}
