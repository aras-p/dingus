#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"

int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
};

SPosCol vsMain0( SInput0 i ) {
	SPosCol o;
	o.pos.w = 1;
	gSkinningPos( i.pos, i.indices, o.pos.xyz );
	o.color.ra = 0;
	o.color.g = o.pos.y / 3.0;
	o.color.b = o.pos.y / 8.0;
	o.pos = mul( o.pos, mViewProj );
	return o;
}

half4 psMain( SPosCol i ) : COLOR {
	return i.color;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_1_1 psMain();
	}
	RESTORE_PASS
}
