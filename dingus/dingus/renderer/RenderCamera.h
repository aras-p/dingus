// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RENDER_CAMERA_H
#define __RENDER_CAMERA_H

#include "../math/Matrix4x4.h"

namespace dingus {


class CRenderCamera {
public:
	CRenderCamera();
	
	void setCameraMatrix( const SMatrix4x4& matrix );
	void setProjectionMatrix( const SMatrix4x4& matrix );
	
	const SMatrix4x4& getCameraMatrix() const { return mCameraMatrix; }
	const SMatrix4x4& getCameraRotMatrix() const { return mCameraRotMatrix; }
	const SMatrix4x4& getViewMatrix() const { return mViewMatrix; }
	const SMatrix4x4& getViewProjMatrix() const { return mViewProjMatrix; }
	const SMatrix4x4& getProjectionMatrix() const { return mProjectionMatrix; };
	const SVector4& getEye() const { return mEye; }
	const SVector3& getEye3() const { return mCameraMatrix.getOrigin(); }
	
private:
	SMatrix4x4		mCameraMatrix;	
	SMatrix4x4		mProjectionMatrix;

	// calculated basing on camera matrix

	SMatrix4x4		mViewMatrix;
	SMatrix4x4		mViewProjMatrix;
	SMatrix4x4		mCameraRotMatrix;
	SVector4		mEye;
};


}; // namespace


#endif
