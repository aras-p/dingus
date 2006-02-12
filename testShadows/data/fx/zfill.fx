#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4 mWVP;

SPos vsMain( SPos i )
{
	SPos o;
	o.pos = mul( i.pos, mWVP );
	return o;
}

half4 psMain() : COLOR
{
	return 0;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
	}
	RESTORE_PASS
}
