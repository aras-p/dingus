#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/dof.fx"

float4x4	mWorld;
float4x4	mWorldView;
float4x4	mWVP;

float3		vLightPosOS;
float3		vEyeOS;


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
	float4 tolight	: COLOR0;		// object space
	float2 uv		: TEXCOORD0;
	half4  halfangz	: TEXCOORD1;	// half-angle object space; camera depth
};


SOutput vsMain( SPosTex i ) {
	SOutput o;

	o.pos = mul( i.pos, mWVP );

	float3 tolight = vLightPosOS - i.pos.xyz;
	tolight = normalize( tolight );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	float3 toview = normalize( vEyeOS - i.pos.xyz );
	o.halfangz.xyz = normalize( tolight + toview );

	o.halfangz.w = gCameraDepth( i.pos, mWorldView );

	o.uv = i.uv;
	
	return o;
}

half4 psMain( SOutput i ) : COLOR {
	// sample normal+AO map
	half4 normalAO = tex2D( smpNormalAO, i.uv );
	half3 normal = normalAO.rgb*2-1;
	normal = normalize( normal );

	half amb = 0.1;
	half ambBias = 0.1;
	half ambMul = 0.9;
#if D_AO==1
	half occ = normalAO.a * ambMul;
#else
	half occ = 1;
#endif

	// calc lighting
#if D_NORMALMAPS==1
	half diffuse = saturate( dot( normal, i.tolight.xyz*2-1 ) );
	float spec = pow( saturate( dot( normal, i.halfangz.xyz ) ), 16 );
#else
	half diffuse = 1;
	float spec = 0;
#endif

	diffuse = diffuse * occ + ambBias;

	// sample diffuse/gloss map
	half4 cBase = tex2D( smpBase, i.uv );
	half3 cDiff = cBase.rgb;
	
	half3 cSpec = lerp( cDiff, half3(1,1,1), 0.5 );
	spec *= cBase.a;

	half3 col = cDiff * diffuse + cSpec * spec + amb;

	return half4( col, gBluriness(i.halfangz.w) );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();

		//FillMode = Wireframe;
	}
	pass PLast {
		//FillMode = Solid;
	}
}



struct SOutputFFP {
	float4 pos		: POSITION;
	float4 tolight	: COLOR0;		// object space
	float2 uv[2]	: TEXCOORD0;
};


SOutputFFP vsMainFFP( SPosTex i ) {
	SOutputFFP o;

	o.pos = mul( i.pos, mWVP );

	float3 tolight = vLightPosOS - i.pos.xyz;
	tolight = normalize( tolight );
	o.tolight = float4( tolight*0.5+0.5, 1 );

	o.uv[0] = i.uv;
	o.uv[1] = i.uv;
	
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFFP();
		PixelShader = NULL;

		Sampler[0] = (smpNormalAO);
		ColorOp[0] = DotProduct3;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;

		Sampler[1] = (smpBase);
		ColorOp[1] = Modulate;
		ColorArg1[1] = Texture;
		ColorArg2[1] = Current;
		AlphaOp[1] = SelectArg1;
		AlphaArg1[1] = Texture;

		//ColorOp[2] = SelectArg2;
		//ColorArg1[2] = Current;
		//ColorArg2[2] = Current | AlphaReplicate;
		//AlphaOp[2] = SelectArg1;
		//AlphaArg1[2] = Current;

		ColorOp[2] = Disable;
		AlphaOp[2] = Disable;
	}
	pass PLast {
	}
}
