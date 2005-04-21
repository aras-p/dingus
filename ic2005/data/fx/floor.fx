#include "lib/shared.fx"
#include "lib/structs.fx"

texture		tShadow;
sampler2D	smpShadow = sampler_state {
	Texture = (tShadow);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


SPosTex vsMain( SPos i ) {
	SPosTex o;
	i.pos = i.pos.xzyw;
	i.pos.xz *= 15;
	
	o.uv = mul( i.pos, mShadowProj0 ).xy;
	o.pos = mul( i.pos, mViewProj );

	return o;
}

float4 psMain( SPosTex i ) : COLOR {
	return tex2D( smpShadow, i.uv );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
	}
}
