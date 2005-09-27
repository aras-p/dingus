#include "lib/structs.fx"
#include "lib/shared.fx"

texture		tBase;
sampler		smpBase = sampler_state {
    Texture   = (tBase);
    MipFilter = None;	MinFilter = Linear; MagFilter = Linear;
    AddressU = Clamp; AddressV = Clamp;
};


SPosTex vsMain11( SPosTex i )
{
    SPosTex o;
	o.pos = i.pos * float4(2,2,1,1);
	o.uv = i.uv + 0.5/SHADOW_MAP_SIZE;
	return o;
}

static const float COEFFS[25] = {
	0.00199005,
	0.00367145,
	0.00642218,
	0.0106512,
	0.016749,
	0.0249718,
	0.0353006,
	0.0473137,
	0.0601261,
	0.0724455,
	0.0827621,
	0.0896446,
	0.0920636,
	0.0896446,
	0.0827621,
	0.0724455,
	0.0601261,
	0.0473137,
	0.0353006,
	0.0249718,
	0.016749,
	0.0106512,
	0.00642218,
	0.00367145,
	0.00199005,
};

half4 psMain20( float2 uv : TEXCOORD0 ) : COLOR {
	half4 col = tex2D( smpBase, uv );
	col.g = 0;
	float texel = 1.0 / SHADOW_MAP_SIZE;
	for( int r = -12; r <= 12; r+=1 ) {
		col.g += tex2D( smpBase, uv + float2(0,texel*r) ).g * COEFFS[r+12];
	}
	return col * 1.1;
}

technique tec20 {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_2_0 psMain20();

		ZEnable = False;
		ZWriteEnable = False;
	}
	RESTORE_PASS
}
