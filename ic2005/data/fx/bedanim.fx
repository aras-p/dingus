#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/skinning.fx"

int			iBones;

struct SInput0 {
	float4	pos		: POSITION;
	float4	indices : BLENDINDICES;
	float3	normal	: NORMAL;
};

SPosColTexp vsMain0( SInput0 i ) {
	SPosColTexp o;
	
	// compensate for lack of UBYTE4 on Geforce3
	o.pos.w = 1;
	int4 indices = D3DCOLORtoUBYTE4( i.indices );
	float4x3 skin = mSkin[indices[0]];

	o.pos.xyz = mul( i.pos, skin ); // world pos
	float3 n = mul( i.normal*2-1, (float3x3)skin );

	float3 tolight = normalize( vLightPos - o.pos.xyz );

	o.uvp = mul( i.pos, mShadowProj );

	float diffuse = max( 0.0, dot( tolight, n ) );
	o.color = diffuse * 0.6 + 0.4;

	o.pos = mul( o.pos, mViewProj );

	return o;
}

half4 psMain( SPosColTexp i ) : COLOR {
	half3 col = tex2Dproj( smpShadow, i.uvp ) * i.color;
	return half4( col, 1 );
}

technique tec20
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMain0();
		PixelShader = compile ps_2_0 psMain();
	}
	pass PLast {
		Texture[0] = NULL;
	}
}
