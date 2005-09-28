#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"


texture		tBase1;
sampler2D	smpBase1 = sampler_state {
	Texture = (tBase1);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};

texture		tBase2;
sampler2D	smpBase2 = sampler_state {
	Texture = (tBase2);
	MagFilter = Linear; MinFilter = Linear; MipFilter = Linear;
};


// --------------------------------------------------------------------------

SPosColTexFog vsMain20( SPosCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	o.color.xyz = 1-abs(n.y)*0.6;
	o.color.w = i.color.w;

	o.uv = float2(i.pos.x,i.pos.z) * 0.02;

	o.fog = gFog( i.pos );

	return o;
}

half4 psMain20( SPosColTexFog i ) : COLOR {
	half4 col1 = tex2D( smpBase1, i.uv );
	half4 col2 = tex2D( smpBase2, i.uv );
	half4 col = lerp( col1, col2, i.color.a );
	col.rgb *= i.color.rgb;
	return col;
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain20();
		PixelShader = compile ps_2_0 psMain20();

		FogEnable = True;

		CullMode = None;
	}
	pass PLast {
		FogEnable = False;
		CullMode = <iCull>;
	}
}

// --------------------------------------------------------------------------

SPosColTexFog vsMainFF( SPosCol i ) {
	SPosColTexFog o;
	o.pos = mul( i.pos, mViewProj );

	float3 n = i.color.xyz*2-1;
	o.color.xyz = 1-abs(n.y)*0.6;
	o.color.w = i.color.w;

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

		CullMode = None;

		Sampler[0] = (smpBase1);

		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	pass PLast {
		FogEnable = False;
		CullMode = <iCull>;
	}
}
