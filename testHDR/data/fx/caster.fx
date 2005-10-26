#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/shadowmap.fx"

// --------------------------------------------------------------------------
//  vertex shader

SPosZ vsMain( SPos i ) {
	SPosZ o;
	o.pos = mul( i.pos, mViewProj );
	o.z = gShadowZ( o.pos );
	return o;
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psCasterMain();
		CullMode = None;
	}
	RESTORE_PASS
}
