#include "stdafx.h"
#include "SceneInteractive.h"
#include "ControllableCharacter.h"
#include "ThirdPersonCamera.h"
#include "SceneShared.h"
#include <dingus/math/Line3.h>
#include <dingus/utils/Random.h>

// --------------------------------------------------------------------------

CSceneInteractive::CSceneInteractive( CSceneSharedStuff* sharedStuff )
:	mSharedStuff( sharedStuff )
,	mAttackStartTime(-1)
,	mWallHitTime(-1)
{
	const float WALK_BOUNDS = 0.9f;
	mCharacter = new CControllableCharacter( ROOM_MIN.x+WALK_BOUNDS, ROOM_MIN.z+WALK_BOUNDS, ROOM_MAX.x-WALK_BOUNDS, ROOM_MAX.z-WALK_BOUNDS );
	addAnimEntity( *mCharacter );

	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// room
	gReadScene( "data/scene.lua", mRoom );

	const float CAMERA_BOUND = 0.15f;
	SVector3 CAMERA_BOUND_MIN = ROOM_MIN + SVector3(CAMERA_BOUND,CAMERA_BOUND,CAMERA_BOUND);
	SVector3 CAMERA_BOUND_MAX = ROOM_MAX - SVector3(CAMERA_BOUND,CAMERA_BOUND,CAMERA_BOUND);
	mCamController = new CThirdPersonCameraController( mCharacter->getWorldMatrix(), getCamera().mWorldMat, CAMERA_BOUND_MIN, CAMERA_BOUND_MAX );
}

CSceneInteractive::~CSceneInteractive()
{
	stl_utils::wipe( mRoom );
	delete mCamController;
}


void CSceneInteractive::update( time_value demoTime, float dt )
{
	mSharedStuff->updatePhysics();

	int n = mRoom.size();
	for( int i = 0; i < n; ++i ) {
		mRoom[i]->update( LIGHT_POS_1 );
	}
	
	float demoTimeS = demoTime.tosec();

	mCharacter->update( demoTime );

	mSharedStuff->updateFracture( 0, demoTimeS );

	mCamController->update( dt );
	const float camnear = 0.1f;
	const float camfar = 50.0f;
	const float camfov = D3DX_PI/4;
	getCamera().setProjectionParams( camfov, CD3DDevice::getInstance().getBackBufferAspect(), camnear, camfar );

	//const float dofDist = SVector3(getCamera().mWorldMat.getOrigin() - getLightTargetMatrix()->getOrigin()).length() * 1.2f;
	//const float dofRange = dofDist * 3.0f;
	const float dofDist = 4.0f;
	const float dofRange = 5.0f;
	gDOFParams.set( dofDist, 1.0f / dofRange, 0.0f );

	// attack must be started now?
	if( mAttackStartTime.value >= 0 && demoTime >= mAttackStartTime ) {
		mAttackStartTime = time_value(-1);
		CConsole::CON_WARNING.write( "atk start" );

		// figure out attack position and direction
		// TBD: right now very simple, just for testing. Waiting for attack fx
		SLine3 atkRay;
		atkRay.pos = mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex].getOrigin()
			+ SVector3(
				gRandom.getFloat(-0.5f,0.5f),
				gRandom.getFloat(0.2f, 0.6f),
				gRandom.getFloat(-0.5f,0.5f) );
		atkRay.vec = -mCharacter->getWorldMatrix().getAxisX() + SVector3(0,0.2f,0);
		float t = mSharedStuff->intersectRay( atkRay );
		if( t > 0.1f && t < 5.0f ) {
			CConsole::CON_WARNING.write( "atk will hit!" );
			mWallHitTime = demoTime + time_value::fromsec(0.3f);
			mWallHitPos = atkRay.pos + atkRay.vec * t;
			mWallHitRadius = (6.0f-t)*0.2f;
		}
	}

	// wall is hit now?
	if( mWallHitTime.value >= 0 && demoTime >= mWallHitTime ) {
		mWallHitTime = time_value(-1);

		mSharedStuff->fractureSphere( demoTimeS, mWallHitPos, mWallHitRadius );
		CConsole::CON_WARNING.write( "hit!" );
	}
}


void CSceneInteractive::render( eRenderMode renderMode )
{
	mSharedStuff->renderWalls( 0, renderMode, false );
	
	mCharacter->render( renderMode );
	
	int i, n;
	n = mRoom.size();
	for( i = 0; i < n; ++i ) {
		mRoom[i]->render( renderMode );
	}
}


void CSceneInteractive::processInput( float mov, float rot, bool attack, time_value demoTime )
{
	mCharacter->move( mov, demoTime );
	mCharacter->rotate( rot );
	if( attack && mAttackStartTime.value < 0 ) {
		mCharacter->attack( demoTime );
		mAttackStartTime = demoTime + time_value::fromsec(0.5f);
	}
}

const SMatrix4x4* CSceneInteractive::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
