#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"
#include "lib/dof.fx"


int			iBones;

struct SInput {
	float4	pos		: POSITION;
	float	weight  : BLENDWEIGHT;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
	float2  uv		: TEXCOORD0;
};

struct SOutput {
	float4 pos		: POSITION;
	float4 toview	: COLOR0;		// world space
	float2 uv		: TEXCOORD0;
	float  z		: TEXCOORD1;
};

SOutput vsMain( SInput i ) {
	SOutput o;

	float3 wpos, wn;
	o.pos.w = 1;
	gSkinning2( i.pos, i.normal, i.indices, i.weight, o.pos.xyz, wn );
	
	o.z = gCameraDepth( o.pos.xyz );

	float3 toview = normalize( vEye - o.pos.xyz );
	o.toview = float4( toview*0.5+0.5, 1 );

	o.pos = mul( o.pos, mViewProj );

	o.uv = i.uv;
	return o;
}

half4 psMain( SOutput i ) : COLOR {
	return half4( i.toview.xyz, gBluriness(i.z) );
}


technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain();
		PixelShader = compile ps_2_0 psMain();
		FillMode = Wireframe;
	}
	pass PLast {
		FillMode = Solid;
	}
}
