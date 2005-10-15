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

/*
SPosColTex2F vsMainFF( SPosCol i ) {
	SPosColTex2F o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	o.color.xyz = 1-abs(n.y)*0.6;
	o.color.w = i.color.w;

	o.uv[0] = o.uv[1] = float2(i.pos.x,i.pos.z) * 0.02;

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

		Sampler[0] = (smpBase1);
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Texture;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		Sampler[1] = (smpBase2);
		ColorOp[1] = BlendCurrentAlpha;
		ColorArg1[1] = Texture;
		ColorArg2[1] = Current;
		AlphaOp[1] = SelectArg1;
		AlphaArg1[1] = Diffuse;

		ColorOp[2] = Modulate;
		ColorArg1[2] = Current;
		ColorArg2[2] = Diffuse;
		AlphaOp[2] = SelectArg1;
		AlphaArg1[2] = Current;

		ColorOp[3] = Disable;
		AlphaOp[3] = Disable;

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
*/