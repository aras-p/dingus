#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float4x4	mWorldView;
float4x4	mWVP;


// --------------------------------------------------------------------------

SPosColFog vsMainFF( SPos i ) {
	SPosColFog o;
	o.pos = mul( i.pos, mWVP );
	o.color = 0;
	o.fog = gFogWV( i.pos, mWorldView );
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		FogEnable = True;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
		FogEnable = False;
	}
}
