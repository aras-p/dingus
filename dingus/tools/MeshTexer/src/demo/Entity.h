#ifndef __DEMO_ENTITY_H
#define __DEMO_ENTITY_H

#include <dingus/math/Matrix4x4.h>


// --------------------------------------------------------------------------

/**
 *  Base entity.
 *  Nothing but a bunch of matrices: world, world*view, world*view*proj.
 */
class CAbstractEntity : public boost::noncopyable {
public:
	CAbstractEntity() { mWorldMat.identify(); mWVMat.identify(); mWVPMat.identify(); }
	virtual ~CAbstractEntity() = 0 { }

	void addMatricesToParams( CEffectParams& ep ) const {
		ep.addMatrix4x4Ref( "mWorld", mWorldMat );
		ep.addMatrix4x4Ref( "mWorldView", mWVMat );
		ep.addMatrix4x4Ref( "mWVP", mWVPMat );
	}

	/// Updates WV/WVP from current matrix and current render camera.
	void updateWVPMatrices() {
		mWVMat = mWorldMat * G_RENDERCTX->getCamera().getViewMatrix();
		mWVPMat = mWorldMat * G_RENDERCTX->getCamera().getViewProjMatrix();
	}

public:
	//SMatrix4x4	mLocalMat;
	SMatrix4x4	mWorldMat;
	SMatrix4x4	mWVMat;
	SMatrix4x4	mWVPMat;
};


// --------------------------------------------------------------------------

/**
 *  Camera entity.
 *  Has projection parameters.
 */
class CCameraEntity : public CAbstractEntity {
public:
	CCameraEntity() { }

	void setProjectionParams( float fov, float aspect, float znear, float zfar );
	void setOrthoParams( float width, float height, float znear, float zfar );
	const SMatrix4x4& getProjectionMatrix() const { return mProjectionMatrix; }
	void setProjFrom( const CCameraEntity& c );

	float getZNear() const { return mZNear; }

	/**
	 *  Gets world space ray from camera's viewing plane coordinates.
	 *  @param x X coordinate (-1 at left, 1 at right).
	 *  @param y Y coordinate (-1 at top, 1 at bottom).
	 *  @return Ray in world space (NOT unit length).
	 */
	dingus::SVector3 getWorldRay( float x, float y ) const;
	/**
	 *  Gets camera space ray from camera's viewing plane coordinates.
	 *  @param x X coordinate (-1 at left, 1 at right).
	 *  @param y Y coordinate (-1 at top, 1 at bottom).
	 *  @return Ray in camera space (NOT unit length).
	 */
	dingus::SVector3 getCameraRay( float x, float y ) const;

	/**
	 *  Set current camera parameters onto rendering context's camera.
	 */
	void	setOntoRenderContext() const;
	
private:
	// projection params
	SMatrix4x4	mProjectionMatrix;
	bool	mOrtho;
	float	mFOV;
	float	mZNear;
	float	mZFar;
	float	mViewHalfWidth; // at distance 1
	float	mViewHalfHeight; // at distance 1
};


#endif
