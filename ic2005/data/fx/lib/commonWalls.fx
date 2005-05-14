#ifndef __COMMON_WALLS_FX
#define __COMMON_WALLS_FX

#include "shared.fx"

float4 gWallLightFull( float3 wpos, float3 wn, float diff, float amb )
{
	float3 tolight = normalize( vLightPos - wpos );
	float diffuse = max( 0.0, dot( tolight, wn ) );
	float col = diffuse * diff + amb;
	return col;
}

float4 gWallLight( float3 wpos, float3 wn )
{
	return gWallLightFull( wpos, wn, 0.6, 0.4 );
}


half gWallLightPS( half3 wn, half3 tol )
{
	half cDiff = 0.6;
	half cAmb = 0.4;
	half diffuse = saturate( dot( tol, wn ) );
	half col = diffuse * cDiff + cAmb;
	return col;
}


#endif
