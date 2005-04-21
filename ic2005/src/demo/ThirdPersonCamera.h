#ifndef __THIRD_PERSON_CAMERA_H
#define __THIRD_PERSON_CAMERA_H



class CThirdPersonCameraController : public boost::noncopyable {
public:
	CThirdPersonCameraController( const SMatrix4x4& charMat, SMatrix4x4& cameraMat,
		const SVector3& boundMin, const SVector3& boundMax );

	void	update( float dt );

private:
	SVector3	getIdealOrigin() const;
	SVector3	getIdealLookat() const;

private:
	SVector3	mBoundMin;
	SVector3	mBoundMax;

	const SMatrix4x4*	mCharMat;
	SMatrix4x4*			mCameraMat;

	SVector3	mLookatPoint;
	SVector3	mLookatVel;
	SVector3	mOriginVel;
};


#endif
