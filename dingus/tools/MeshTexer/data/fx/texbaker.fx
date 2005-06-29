#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/meshtexer.fx"

float4x4	mWorld;

float		fSizeX;
float		fSizeY;


texture		tBase;
sampler2D	smpBase = sampler_state {
	Texture = (tBase);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

texture		tNormalAO;
sampler2D	smpNormalAO = sampler_state {
	Texture = (tNormalAO);
	MinFilter = Linear; MagFilter = Linear; MipFilter = Linear;
	AddressU = Wrap; AddressV = Wrap;
};

struct SOutput {
	float4 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float3 opos		: TEXCOORD1;
};

SOutput vsMain( SPosTex i, uniform float xo, uniform float yo ) {
	SOutput o;

	float3 wpos = mul( i.pos, mWorld );
	o.opos = wpos;

	// render into UV space
	o.pos = float4( i.uv*2-1, 0.5, 1.0 );
	o.pos.y = -o.pos.y;
	o.pos.x += (xo - 1) / fSizeX;
	o.pos.y += (yo + 1) / fSizeY;

	o.uv = i.uv;

	return o;
}


half4 psMain( SOutput i ) : COLOR {
	// sample normal map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;
	normal = normalize( normal );

	half3 scales = abs(normal);
	scales *= scales;

	// construct diffuse map
	half3 cDiff = gMapDiffuse( i.opos, smpBase, scales );

	return half4( cDiff, 1 );
}


technique tec0
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain( -4.0, -4.0 );
		PixelShader = compile ps_2_0 psMain();
		CullMode = None;
		FillMode = Solid;
	}
	pass P1 {
		VertexShader = compile vs_1_1 vsMain( 4.0, 4.0 );
	}
	pass P2 {
		VertexShader = compile vs_1_1 vsMain( -4.0, 4.0 );
	}
	pass P3 {
		VertexShader = compile vs_1_1 vsMain( 4.0, -4.0 );
	}
	pass P4 {
		VertexShader = compile vs_1_1 vsMain( -2.0, -2.0 );
	}
	pass P5 {
		VertexShader = compile vs_1_1 vsMain( 2.0, 2.0 );
	}
	pass P6 {
		VertexShader = compile vs_1_1 vsMain( -2.0, 2.0 );
	}
	pass P7 {
		VertexShader = compile vs_1_1 vsMain( 2.0, -2.0 );
	}
	pass P8 {
		VertexShader = compile vs_1_1 vsMain( 0, 0 );
	}
	pass PLast {
		CullMode = <iCull>;
		FillMode = <iFill>;
	}
}
