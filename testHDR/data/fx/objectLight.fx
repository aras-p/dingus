#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/hdrlib.fx"

float3 vPos;


// --------------------------------------------------------------------------
//  vertex shader

SPos vsMain( SPos i ) {
	SPos o;
	const float MUL_FACTOR = 20.0;
	i.pos.xyz *= MUL_FACTOR;
	i.pos.xyz += vPos;
	o.pos = mul( i.pos, mViewProj );
	o.pos.z /= MUL_FACTOR; // make it appear nearer
	return o;
}


half4 psMain() : COLOR
{
	float3 color = fLightIntensity * 20;
	return EncodeRGBE8( color );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		ZEnable = False;
		ZWriteEnable = False;
	}
	RESTORE_PASS
}
