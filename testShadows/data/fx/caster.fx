#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/shadowmap.fx"

float4x4 mWVP;

SPosZ vsMain( SPos i )
{
	SPosZ o;
	o.pos = mul( i.pos, mWVP );
	o.z = gShadowZ( o.pos );
	return o;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psCasterMain();
	}
	RESTORE_PASS
}
