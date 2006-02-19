// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "GfxUtils.h"

using namespace dingus;


void gfx::textureProjectionWorld( const SMatrix4x4& projectorMatrix, float texX, float texY, SMatrix4x4& dest )
{
    // | -0.5     0        0        0 |
    // | 0        0.5      0        0 |
    // | 0        0        1        0 |
    // | 0.5      0.5      0        1 |
	SMatrix4x4 matTexScale;
	matTexScale.identify();
	matTexScale._11 = 0.5f;
	matTexScale._22 = -0.5f;
	matTexScale._33 = 1.0f;
	float xo =  0.5f / texX;
	float yo =  0.5f / texY;
	matTexScale.getOrigin().set( 0.5f + xo, 0.5f + yo, 0.0f );

	dest = projectorMatrix * matTexScale;
}


void gfx::textureProjectionView( const SMatrix4x4& renderCameraMatrix, const SMatrix4x4& projectorMatrix, float texX, float texY, SMatrix4x4& dest )
{
    // | -0.5     0        0        0 |
    // | 0        0.5      0        0 |
    // | 0        0        1        0 |
    // | 0.5      0.5      0        1 |
	SMatrix4x4 matTexScale;
	matTexScale.identify();
	matTexScale._11 = 0.5f;
	matTexScale._22 = -0.5f;
	matTexScale._33 = 1.0f;
	float xo =  0.5f / texX;
	float yo =  0.5f / texY;
	matTexScale.getOrigin().set( 0.5f + xo, 0.5f + yo, 0.0f );

	dest = renderCameraMatrix * projectorMatrix * matTexScale;
}



void gfx::clippedProjection(
	const SMatrix4x4& view, const SMatrix4x4& proj, const SPlane& clipPlane,
	SMatrix4x4& clippedProj )
{
	SMatrix4x4 viewProj = view * proj;

	SPlane normPlane;
	D3DXPlaneNormalize( &normPlane, &clipPlane );
	
	// inverse transpose of viewproj
	D3DXMatrixInverse( &viewProj, NULL, &viewProj );
	D3DXMatrixTranspose( &viewProj, &viewProj );
	
	// transform clip plane into projection space
	SVector4 clipPlaneV( normPlane.a, normPlane.b, normPlane.c, normPlane.d );
	SVector4 projClipPlaneV;
	D3DXVec4Transform( &projClipPlaneV, &clipPlaneV, &viewProj );

	if( fabsf( projClipPlaneV.w ) < 1.0e-6f ) {
		// plane is perpendicular to the near plane
		clippedProj = proj;
		return;
	}
	
	clippedProj.identify();
	
	if( projClipPlaneV.w > 0.0f ) {
		// flip plane to point away from eye
		clipPlaneV = -clipPlaneV;
		D3DXVec4Transform( &projClipPlaneV, &clipPlaneV, &viewProj );
	}
	
	// put projection space clip plane in Z column
	clippedProj(0,2) = projClipPlaneV.x;
	clippedProj(1,2) = projClipPlaneV.y;
	clippedProj(2,2) = projClipPlaneV.z;
	clippedProj(3,2) = projClipPlaneV.w;
	
	// multiply into projection matrix
	clippedProj = proj * clippedProj;
}
