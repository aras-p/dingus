#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/commonWalls.fx"

float4x4	mWorld;
float4x4	mWVP;
float3		vNormal;

float		fAlpha = 0.5;


SPosCol vsMain( SPosN i ) {
	SPosCol o;
	float3 wpos = mul( i.pos, mWorld );
	o.pos = mul( i.pos, mWVP );
	o.color = gWallLight( wpos, vNormal );
	o.color.a = fAlpha;
	return o;
}

half4 psMain( SPosCol i ) : COLOR {
	return i.color;
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
		Texture[0] = NULL;
		AlphaBlendEnable = False;
		ZWriteEnable = True;
	}
}
