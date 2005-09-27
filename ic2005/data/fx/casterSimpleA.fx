#include "lib/shared.fx"
#include "lib/structs.fx"


SPosCol vsMain( SPosCol i ) {
	SPosCol o;
	o.pos = mul( i.pos, mViewProj );
	o.color.xyz = 0.6;
	o.color.w = i.color.w;
	return o;
}

half4 psMain( SPosCol i ) : COLOR {
	return i.color;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_1_1 psMain();
		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
	}
	RESTORE_PASS
}
