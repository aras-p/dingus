#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/shadowmap.fx"

int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
};

SPosZ vsMain0( SInput0 i ) {
	SPosZ o;
	o.pos.w = 1;
	gSkinningPos( i.pos, i.indices, o.pos.xyz );
	o.pos = mul( o.pos, mViewProj );

	o.z = gShadowZ( o.pos );
	return o;
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psCasterMain();
		CullMode = CCW;
	}
	pass PLast {
		CullMode = <iCull>;
	}
}
