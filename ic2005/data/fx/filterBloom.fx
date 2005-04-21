//
// Pyramid filter for blurring. Repeatedly applied produces approximate
// Gaussian blur. As presented by M.Kawase in GDC2003, "Frame Buffer
// Postprocessing Effects"
//
// ps1.1
// 

#include "lib/structs.fx"
  
texture		tBase;
sampler		smpBase = sampler_state {
    Texture   = (tBase);
    MipFilter = None;	MinFilter = Linear; MagFilter = Linear;
    AddressU = Clamp; AddressV = Clamp;
};

// x=0.5/texwidth, y=0.5/texheight, z=1-x*2, w=1-y*2
float4	vFixUV;
float4	vTexelsX;
float4	vTexelsY;


SPosTex4 vsMain11( SPosTex i )
{
    SPosTex4 o;
	o.pos = i.pos * float4(2,2,1,1);
	float2 uv = i.uv + vFixUV.xy;
	for( int i = 0; i < 4; ++i )
		o.uv[i] = uv + float2( vTexelsX[i], vTexelsY[i] );
	return o;
}

half4 psMain11( SPosTex4 i ) : COLOR
{
	int t;
	half4 c = 0;
	for( t = 0; t < 4; ++t ) {
		half4 cc = tex2D( smpBase, i.uv[t] );
		c += cc/4;
	}
	return c;
}

technique tec11 {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_1_1 psMain11();

		ZEnable = False;
		ZWriteEnable = False;
	}
	pass PLast {
		Texture[0] = NULL;
		Texture[1] = NULL;
		Texture[2] = NULL;
		Texture[3] = NULL;
		ZEnable = True;
		ZWriteEnable = True;
	}
}
