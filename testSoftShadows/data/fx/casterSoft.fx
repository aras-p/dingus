#include "lib/shared.fx"
#include "lib/structs.fx"


float4x4 mWorld;
float4x4 mWVP;


float evalDistance( float3 p ) {
	return p.y;
}

/*
float evalDistance( float3 p ) {
	float dpx = vMax.x - p.x;
	float dnx = p.x - vMin.x;
	float dx = min( dpx, dnx );

	float dpz = vMax.z - p.z;
	float dnz = p.z - vMin.z;
	float dz = min( dpz, dnz );

	float dny = p.y - vMin.y;

	float dist = min( min( dx, dz ), dny );
	return max( 0, dist );
}*/

/*
float evalDistance( float3 p ) {
    float3 toMax = vMax - p;
	float3 toMin = p - vMin;
	float3 d = min( toMax, toMin );
	return min( min( d.x, d.y ), d.z );
}
*/

SPosCol vsMain11( SPos i ) {
	SPosCol o;
	float3 wpos = mul( i.pos, mWorld );
	o.pos = mul( i.pos, mWVP );
	
	float dist = evalDistance( wpos );

	o.color.ra = 0;
	o.color.g = dist * 0.3;
	o.color.b = dist * 0.1;

	return o;
}


technique tecFFP {
	pass P0 {
		VertexShader = compile vs_1_1 vsMain11();
		PixelShader = NULL;
		CullMode = None;

		ColorOp[0]	 = SelectArg1;
		ColorArg1[0] = Diffuse;
		AlphaOp[0]	 = SelectArg1;
		AlphaArg1[0] = Diffuse;
		ColorOp[1]	 = Disable;
		AlphaOp[1]	 = Disable;
	}
	pass PLast {
		CullMode = <iCull>;
	}
}
