// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "RenderCamera.h"

using namespace dingus;


CRenderCamera::CRenderCamera()
:	mEye(0,0,0,1)
{
	mCameraMatrix.identify();
	mProjectionMatrix.identify();
	mCameraRotMatrix.identify();
	mViewMatrix.identify();
	mViewMatrix.identify();
}


void CRenderCamera::setCameraMatrix( SMatrix4x4 const& matrix )
{
	mCameraMatrix = matrix;
	mCameraRotMatrix = matrix;
	mCameraRotMatrix.getOrigin().set(0,0,0);
	D3DXMatrixInverse( &mViewMatrix, NULL, &mCameraMatrix );
	mEye = SVector4( matrix._41, matrix._42, matrix._43, 1.0f );
	mViewProjMatrix = mViewMatrix * mProjectionMatrix;
}

void CRenderCamera::setProjectionMatrix( SMatrix4x4 const& matrix )
{
	mProjectionMatrix = matrix;
	mViewProjMatrix = mViewMatrix * mProjectionMatrix;
}
