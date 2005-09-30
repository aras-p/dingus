#include "lib/shared.fx"
#include "lib/structs.fx"
#include "lib/fog.fx"

float4x4	mWorldView;
float4x4	mWVP;
float4		vColor;



// --------------------------------------------------------------------------

SPosCol vsMainFF( SPosN i ) {
	SPosCol o;
	o.pos = mul( i.pos, mWVP );
	float3 vn = mul( i.normal, (float3x3)mWorldView );
	float rim = 1 - abs(vn.z);
	o.color.xyz = vColor.xyz;
	o.color.a = rim*0.75*vColor.a;

	o.color.a *= saturate( gFogWV( i.pos, mWorldView ) );
	return o;
}


technique tecFFP
{
	pass P0 {
		VertexShader = compile vs_1_1 vsMainFF();
		PixelShader = NULL;

		AlphaBlendEnable = True;
		SrcBlend = SrcAlpha;
		DestBlend = One;
		ZWriteEnable = False;
		CullMode = None;

		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;

		ColorOp[1] = Disable;
		AlphaOp[1] = Disable;
	}
	RESTORE_PASS
}
