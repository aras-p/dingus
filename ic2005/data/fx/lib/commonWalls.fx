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


#endif
