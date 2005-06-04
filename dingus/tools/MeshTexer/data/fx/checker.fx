#include "lib/shared.fx"
#include "lib/structs.fx"

float4x4	mWVP;


texture		tChecker;
sampler2D	smpChecker = sampler_state {
	Texture = (tChecker);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};


SPosTex vsMain( SPosTex i ) {
	SPosTex o;
	
	o.pos = mul( i.pos, mWVP );
	o.uv = i.uv;

	return o;
}

half4 psMain( SPosTex i ) : COLOR {
	return tex2D( smpChecker, i.uv );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
	}
	pass PLast {
	}
}
