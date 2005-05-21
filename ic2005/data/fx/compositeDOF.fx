//
// Composite Depth-of-Field using two blurred images and framebuffer.
// As presented by M.Kawase in GDC2003, "Frame Buffer Postprocessing Effects"
//

#include "lib/structs.fx"
#include "lib/shared.fx"

texture		tBase;
texture		tBlur1;
texture		tBlur2;


sampler smpBase = sampler_state {
    Texture   = (tBase);
    MipFilter = Point;	MinFilter = Point; MagFilter = Point;
    AddressU = Clamp; AddressV = Clamp;
};
sampler smpBlur1 = sampler_state {
    Texture   = (tBlur1);
    MipFilter = Point;	MinFilter = Linear; MagFilter = Linear;
    AddressU = Clamp; AddressV = Clamp;
};
sampler smpBlur2 = sampler_state {
    Texture   = (tBlur2);
    MipFilter = Point;	MinFilter = Linear; MagFilter = Linear;
    AddressU = Clamp; AddressV = Clamp;
};


SPosTex3 vsMain( SPosTex i )
{
    SPosTex3 o;
	o.pos = i.pos * float4(2,2,1,1);
	o.uv[0] = i.uv + vScreenFixUV.xy;
	o.uv[1] = i.uv + vScreenFixUV.xy;
	o.uv[2] = i.uv + vScreenFixUV.xy;
	return o;
}

half4 psMain20( SPosTex3 i ) : COLOR
{
	half4 cbase = tex2D( smpBase, i.uv[0] );
	half ca = cbase.a;
	half3 cblur1 = tex2D( smpBlur1, i.uv[1] ).rgb;
	half3 cblur2 = tex2D( smpBlur2, i.uv[2] ).rgb;

	half a;
	if( ca > 0.5 )
		a = saturate( ca * 0.25 + 0.75 );
	else
		a = saturate( ca * 1.5 );
	half factor = saturate( ca * 1.5 - 0.75 );
	half3 blur = lerp( cblur1, cblur2, factor );
	half3 col = lerp( cbase, blur, a );
	return half4( col, 1 );
}

technique tec20 {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain20();

		ZEnable = False;
		ZWriteEnable = False;
		//AlphaBlendEnable = True;
		//SrcBlend = SrcAlpha;
		//DestBlend = InvSrcAlpha;
	}
	pass PLast {
		ZEnable = True;
		ZWriteEnable = True;
		//AlphaBlendEnable = False;
		Texture[0] = NULL;
		Texture[1] = NULL;
	}
}
