// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "ParticleRenderHelper.h"
#include "../../math/Constants.h"

using namespace dingus;

// --------------------------------------------------------------------------

void CParticleRenderHelper::begin( const SMatrix4x4& camRotMatrix, float particleSize )
{
	const float halfSize = particleSize * 0.5f;
	D3DXVec3TransformCoord( &mTransformedCorners[0], &SVector3( -halfSize,  halfSize, 0 ), &camRotMatrix );
	D3DXVec3TransformCoord( &mTransformedCorners[1], &SVector3(  halfSize,  halfSize, 0 ), &camRotMatrix );
	D3DXVec3TransformCoord( &mTransformedCorners[2], &SVector3(  halfSize, -halfSize, 0 ), &camRotMatrix );
	D3DXVec3TransformCoord( &mTransformedCorners[3], &SVector3( -halfSize, -halfSize, 0 ), &camRotMatrix );
}


// --------------------------------------------------------------------------


const float CRotParticleRenderHelper::SPIN2INDEX = (float)(CRotParticleRenderHelper::ROTATIONS_COUNT) / 2.0f / 3.1415926f;

CRotParticleRenderHelper::CRotParticleRenderHelper()
{
	// precalc untransformed corners
	SVector3 v0( -0.5f, -0.5f, 0.0f );
	SVector3 v1(  0.5f, -0.5f, 0.0f );
	SVector3 v2(  0.5f,  0.5f, 0.0f );
	SVector3 v3( -0.5f,  0.5f, 0.0f );
	SMatrix4x4 mat;

	float rad = 0.0f;
	float drad = 2.0f * PY / (float)(ROTATIONS_COUNT);
    for( int i = 0; i < ROTATIONS_COUNT; ++i, rad += drad ) {
		D3DXMatrixRotationZ( &mat, rad );
		D3DXVec3TransformCoord( &mCorners[i][0], &v0, &mat );
		D3DXVec3TransformCoord( &mCorners[i][1], &v1, &mat );
		D3DXVec3TransformCoord( &mCorners[i][2], &v2, &mat );
		D3DXVec3TransformCoord( &mCorners[i][3], &v3, &mat );
	}
}

void CRotParticleRenderHelper::begin( const SMatrix4x4& camRotMatrix )
{
    // precalc transformed corners
	D3DXVec3TransformCoordArray( &mTransformedCorners[0][0], sizeof(SVector3), &mCorners[0][0], sizeof(SVector3), &camRotMatrix, ROTATIONS_COUNT*CORNERS_COUNT );
}


// --------------------------------------------------------------------------

/*
void CLineParticleHelper::begin( const SVector3& cameraPosition )
{
	mCameraPosition = cameraPosition;
}

SVector3 CLineParticleHelper::getPerpendicular( const SVector3& position, const SVector3& direction, float width )
{
	D3DXVec3Cross( &mV, &( position - mCameraPosition ), &direction );

	float l = D3DXVec3Length( &mV );

	if( l > 0 ) mV *= width / l;

	return mV;
}

SVector3 CLineParticleHelper::getStripPerpendicular( const SVector3& position, const SVector3& direction, float width )
{
	D3DXVec3Cross( &mV, &( position - mCameraPosition ), &direction );

	float l = D3DXVec3Length( &mV );

	if( l < 0.01 ) return mPrevP;
	if( l > 0 ) {
		mV *= width / l;

		if( D3DXVec3Dot( &mV, &mPrevP ) < 0 ) mV = -mV;
		mPrevP = mV;
	}

	return mV;
}
*/