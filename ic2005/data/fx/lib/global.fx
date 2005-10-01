// global effect as used by the framework

#include "shared.fx"

technique tec0 {
	pass P0 {
		VertexShader = NULL;
		PixelShader = NULL;
		AlphaBlendEnable = False;
		CullMode = <iCull>;
		ZFunc = LessEqual;
	}
	pass PLast {
	}
}
