#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

float4x4	mWorld;
float4x4	mWVP;
float3		vNormal;

texture		tAlpha;
sampler2D	smpAlpha = sampler_state {
	Texture = (tAlpha);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Clamp; AddressV = Clamp;
};


SPosColTex vsMain( SPosNTex i ) {
	SPosColTex o;
	float3 wpos = mul( i.pos, mWorld );
	o.pos = mul( i.pos, mWVP );
	o.color = gWallLight( wpos, vNormal ) * 1.1;
	o.uv.x = 1-i.uv.x;
	o.uv.y = 1-i.uv.y;
	return o;
}

half4 psMain( SPosColTex i ) : COLOR {
	half a = tex2D( smpAlpha, i.uv ).a;
	return half4(i.color.rgb,a);
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = False;
	}
	pass PLast {
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
