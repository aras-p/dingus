#include "lib/structs.fx"
  
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
	o.uv = i.uv + 0.5/1024;
	return o;
}

half4 psMain20( float2 tc0 : TEXCOORD0 ) : COLOR
{
	half4 col = tex2D( smpBase, tc0 );

	// Alter B channel only.
	// Approx. dilation into non-shadowed regions:
	// Take max of span's G value. Use it only
	// if our pixel is not in shadow (R!=0).

	const int SPANLEN = 7;
	const float texel = 1.0 / 1024;
	
	half maxB = 1;
	for( int k = -SPANLEN; k <= SPANLEN; ++k ) {
		half4 tap = tex2D( smpBase, tc0 + float2(k*texel,0) );
		maxB = min( maxB, tap.g );
	}

	col.g = (col.r >= 0.5) ? maxB : col.g;
	return col;
}


technique tec20 {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_2_0 psMain20();

		ZEnable = False;
		ZWriteEnable = False;
	}
	pass PLast {
		//Texture[0] = NULL;
		ZEnable = True;
		ZWriteEnable = True;
	}
}
