#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/shadowmap.fx"

float4x4 mWVP;

#ifdef DST_SHADOWS
SPos vsMain( SPos i )
{
	SPos o;
	o.pos = mul( i.pos, mWVP );
	return o;
}
#else
SPosZ vsMain( SPos i )
{
	SPosZ o;
	o.pos = mul( i.pos, mWVP );
	o.z = gShadowZ( o.pos );
	return o;
}
#endif

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		#ifdef DST_SHADOWS
		PixelShader = compile ps_2_0 psCasterMain();
		#else
		PixelShader = compile ps_1_1 psCasterMain();
		#endif
	}
	RESTORE_PASS
}
