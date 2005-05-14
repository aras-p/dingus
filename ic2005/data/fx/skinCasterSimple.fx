#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"

int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
};

SPos vsMain0( SInput0 i ) {
	SPos o;
	o.pos.w = 1;
	gSkinningPos( i.pos, i.indices, o.pos.xyz );
	o.pos = mul( o.pos, mViewProj );
	return o;
}

half4 psMain( SPosCol i ) : COLOR {
	return 0.7;
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_1_1 psMain();
	}
	pass PLast {
	}
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = NULL;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
	}
}
