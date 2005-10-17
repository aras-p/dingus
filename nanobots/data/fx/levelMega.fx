#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

SPosColTexFog vsMain11( SPosNCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.normal*2-1;

	o.color = i.color;
	o.color.xyz *= 1.2-abs(n.y)*0.8;

	o.uv = float2(i.pos.x,i.pos.z) * 0.02;

	o.fog = gFog( i.pos );

	return o;
}

half4 psMain11( SPosColTexFog i ) : COLOR {
	half4 col = tex2D( smpBase, i.uv );
	col.rgb *= i.color.rgb;
	return col;
}


technique tec11
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = compile ps_1_1 psMain11();

		FogEnable = True;

		CullMode = CW;

		// set bit 1 where we render
		StencilEnable = True;
		StencilFunc = Always;
		StencilRef = 1;
		StencilMask = 1;
		StencilWriteMask = 1;
		StencilFail = Keep;
		StencilPass = Replace;
		StencilZFail = Keep;
	}
	pass P1 {
		CullMode = CCW;

		// clear bit 1 where we render
		StencilRef = 0;
	}
	RESTORE_PASS
}

// --------------------------------------------------------------------------

SPosColTexFog vsMainFF( SPosNCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.normal*2-1;

	o.color = i.color;
	o.color.xyz *= 1.2-abs(n.y)*0.8;

	o.uv = float2(i.pos.x,i.pos.z) * 0.02;

	o.fog = gFog( i.pos );

	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		FogEnable = True;

		CullMode = CW;

		Sampler[0] = (smpBase);
		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;

		// set bit 1 where we render
		StencilEnable = True;
		StencilFunc = Always;
		StencilRef = 1;
		StencilMask = 1;
		StencilWriteMask = 1;
		StencilFail = Keep;
		StencilPass = Replace;
		StencilZFail = Keep;
	}
	pass P1 {
		CullMode = CCW;
		// clear bit 1 where we render
		StencilRef = 0;
	}
	RESTORE_PASS
}
