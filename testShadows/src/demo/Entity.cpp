#include "stdafx.h"
#include "Entity.h"


// --------------------------------------------------------------------------

CCameraEntity::CCameraEntity()
{
}

void CCameraEntity::updateViewCone()
{
	mViewCone.axis = mWorldMat.getAxisZ();
}

void CCameraEntity::updateViewConeAngleFull()
{
	ConeMake( getCameraRay(-1.0f,-1.0f).getNormalized(), getCameraRay(1.0f,1.0f).getNormalized(), mViewCone );
}

void CCameraEntity::updateViewConeAngleFOV()
{
	mViewCone.cosAngle = cosf( mFOV * 0.5f );
	mViewCone.UpdateAngle();
}

void CCameraEntity::setProjectionParams( float fov, float aspect, float znear, float zfar )
{
	mOrtho = false;
	D3DXMatrixPerspectiveFovLH( &mProjectionMatrix, fov, aspect, znear, zfar );
	mFOV = fov;
	mZNear = znear;
	mZFar = zfar;
	mViewHalfWidth = 1.0f / mProjectionMatrix._11;
	mViewHalfHeight = 1.0f / mProjectionMatrix._22;
}

/*
void CCameraEntity::setOrthoParams( float width, float height, float znear, float zfar )
{
	mOrtho = true;
	D3DXMatrixOrthoLH( &mProjectionMatrix, width, height, znear, zfar );
	mZNear = znear;
	mZFar = zfar;
	mViewHalfWidth = width*0.5f;
	mViewHalfHeight = height*0.5f;
}
*/

void CCameraEntity::setProjFrom( const CCameraEntity& c )
{
	mOrtho = c.mOrtho;
	mFOV = c.mFOV;
	mZNear = c.mZNear;
	mZFar = c.mZFar;
	mViewHalfWidth = c.mViewHalfWidth;
	mViewHalfHeight = c.mViewHalfHeight;
	mProjectionMatrix = c.mProjectionMatrix;
}

void CCameraEntity::setOntoRenderContext() const
{
	// set camera params
	G_RENDERCTX->getCamera().setCameraMatrix( mWorldMat );
	G_RENDERCTX->getCamera().setProjectionMatrix( mProjectionMatrix );
}

dingus::SVector3 CCameraEntity::getWorldRay( float x, float y ) const
{
	dingus::SMatrix4x4 m = mWorldMat;
	m.getOrigin().set(0,0,0);
	dingus::SVector3 r = getCameraRay( x, y );
	D3DXVec3TransformCoord( &r, &r, &m );
	return r;
}

dingus::SVector3 CCameraEntity::getCameraRay( float x, float y ) const
{
	return dingus::SVector3( x*mViewHalfWidth, -y*mViewHalfHeight, 1.0f );
}
