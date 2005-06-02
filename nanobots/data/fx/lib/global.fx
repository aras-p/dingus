// global effect as used by the framework

#include "shared.fx"

technique tec0 {
	pass P0 {
		AlphaBlendEnable = False;
		CullMode = <iCull>;
		ZFunc = Less;

		FogTableMode = None;
		FogVertexMode = Linear;
		FogColor = (vFogColor);
	}
	pass PLast {
	}
}
