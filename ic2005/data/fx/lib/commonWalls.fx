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
	return gWallLightFull( wpos, wn, lpos, 0.70, 0.30 );
}

#define DEF_WALL_DIFF 0.60
#define DEF_WALL_AMB 0.20

half gWallLightPS( half3 wn, half3 tol, half diff, half amb )
{
	half diffuse = saturate( dot( tol, wn ) );
	half col = diffuse * diff + amb;
	return col;
}


#endif
