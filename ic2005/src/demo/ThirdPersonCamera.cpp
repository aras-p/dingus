#include "stdafx.h"
#include "ThirdPersonCamera.h"
#include <dingus/math/MathUtils.h>


CThirdPersonCameraController::CThirdPersonCameraController(
		const SMatrix4x4& charMat, SMatrix4x4& cameraMat,
		const SVector3& boundMin, const SVector3& boundMax
)
:	mBoundMin(boundMin)
,	mBoundMax(boundMax)
,	mCharMat( &charMat )
,	mCameraMat( &cameraMat )
,	mLookatVel(0,0,0)
,	mOriginVel(0,0,0)
{
	mLookatPoint = getIdealLookat();	
}

SVector3 CThirdPersonCameraController::getIdealOrigin() const
{
	const float FWD = -3.0f;
	const float HGT = 1.5f;
	SVector3 p = mCharMat->getOrigin() - mCharMat->getAxisX()*FWD + SVector3(0,HGT,0);
	D3DXVec3Minimize( &p, &p, &mBoundMax );
	D3DXVec3Maximize( &p, &p, &mBoundMin );
	return p;
}

SVector3 CThirdPersonCameraController::getIdealLookat() const
{
	const float FWD = 3.0f;
	const float HGT = 1.4f;
	return mCharMat->getOrigin() - mCharMat->getAxisX()*FWD + SVector3(0,HGT,0);
}


void CThirdPersonCameraController::update( float dt )
{
	SVector3 targetLookat = getIdealLookat();
	SVector3 targetOrigin = getIdealOrigin();

	SVector3 lookat = smoothCD( mLookatPoint, targetLookat, mLookatVel, 0.5f, dt );
	SVector3 origin = smoothCD( mCameraMat->getOrigin(), targetOrigin, mOriginVel, 0.8f, dt );
	mLookatPoint = lookat;

	mCameraMat->getOrigin() = origin;
	mCameraMat->getAxisZ() = SVector3(lookat-origin).getNormalized();
	mCameraMat->getAxisY().set( 0, 1, 0 );
	//mCameraMat->getAxisX() = mCameraMat->getAxisZ().cross( mCameraMat->getAxisY() );
	mCameraMat->getAxisX() = mCameraMat->getAxisY().cross( mCameraMat->getAxisZ() );
}
