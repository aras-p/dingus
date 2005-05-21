#ifndef __COMMON_WALLS_FX
#define __COMMON_WALLS_FX

#include "shared.fx"

float4 gWallLightFull( float3 wpos, float3 wn, float3 lpos, float diff, float amb )
{
	float3 tolight = normalize( lpos - wpos );
	float diffuse = max( 0.0, dot( tolight, wn ) );
	float col = diffuse * diff + amb;
	return col;
}

float4 gWallLight( float3 wpos, float3 wn, float3 lpos )
{
	return gWallLightFull( wpos, wn, lpos, 0.60, 0.40 );
}


half gWallLightPS( half3 wn, half3 tol )
{
	half cDiff = 0.45;
	half cAmb = 0.40;
	half diffuse = saturate( dot( tol, wn ) );
	half col = diffuse * cDiff + cAmb;
	return col;
}


#endif
