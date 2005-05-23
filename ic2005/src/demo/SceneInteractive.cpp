#include "stdafx.h"
#include "SceneInteractive.h"
#include "ControllableCharacter.h"
#include "ThirdPersonCamera.h"
#include "SceneShared.h"
#include <dingus/math/Line3.h>
#include <dingus/utils/Random.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>


// --------------------------------------------------------------------------

CSceneInteractive::CSceneInteractive( CSceneSharedStuff* sharedStuff )
:	mSharedStuff( sharedStuff )
,	mAttackIndex(-1)
,	mAttackStartTime(-1)
,	mAttackAnimStartTime(-1)
,	mWallHitTime(-1)
{
	const float WALK_BOUNDS = 0.9f;
	mCharacter = new CControllableCharacter( ROOM_MIN.x+WALK_BOUNDS, ROOM_MIN.z+WALK_BOUNDS, ROOM_MAX.x-WALK_BOUNDS, ROOM_MAX.z-WALK_BOUNDS );
	addAnimEntity( *mCharacter );

	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );
	mHandLIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "L Finger2" );
	mHandRIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "R Finger2" );

	// room
	gReadScene( "data/scene.lua", mRoom );

	// attacks
	mAttack1L = new CComplexStuffEntity( "AttackFx1", NULL, "AttackFx1L" );
	addAnimEntity( *mAttack1L );
	mAttack1R = new CComplexStuffEntity( "AttackFx1", NULL, "AttackFx1R" );
	addAnimEntity( *mAttack1R );
	mAttack2_1 = new CComplexStuffEntity( "AttackFx2_1", NULL, "Attack_v02Fx" );
	addAnimEntity( *mAttack2_1 );
	mAttack2_2 = new CComplexStuffEntity( "AttackFx2_2", NULL, "Attack_v02Fx" );
	addAnimEntity( *mAttack2_2 );

	mMatLBegin = SMatrix4x4( SVector3(-1.700f,1.320f,-2.725f), SQuaternion(-0.668644f,0.455175f,0.530113f,0.254382f) );
	mMatLMid   = SMatrix4x4( SVector3(-2.144f,1.953f,-2.719f), SQuaternion(-0.0927577f,0.308475f,0.769266f,0.551787f) );
	mMatRBegin = SMatrix4x4( SVector3(-2.088f,0.945f,-2.473f), SQuaternion(0.529337f,-0.699213f,-0.11135f,-0.467445f) );
	mMatRMid   = SMatrix4x4( SVector3(-1.779f,1.597f,-2.707f), SQuaternion(-0.257726f,0.329951f,0.476417f,0.773134f) );

	D3DXMatrixInverse( &mInvMatLBegin, NULL, &mMatLBegin );
	D3DXMatrixInverse( &mInvMatLMid, NULL, &mMatLMid );
	D3DXMatrixInverse( &mInvMatRBegin, NULL, &mMatRBegin );
	D3DXMatrixInverse( &mInvMatRMid, NULL, &mMatRMid );

	// camera
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


void CSceneInteractive::start( time_value demoTime, CUIDialog& dlg )
{
	mSharedStuff->clearPieces();
}


void CSceneInteractive::animateAttack1Bolt( float animTime,
		const SMatrix4x4& handMat, SMatrix4x4* mats, float ts, float th, float addY )
{
	const int MATRIX_COUNT = 4;

	if( ts < 0.0f ) {
		// make very small
		SMatrix4x4 mbase;
		mbase.identify();
		mbase.getAxisX().x = mbase.getAxisY().y = mbase.getAxisZ().z = 1.0e-6f;
		mbase.getOrigin() = handMat.getOrigin();
		for( int i = 0; i < MATRIX_COUNT; ++i ) {
			mats[i] = mbase;
		}
		return;
	}

	// figure out wanted length of the bold and direction lerper
	float wantedLength;
	float dirLerp;
	const float MIN_LENGTH = 0.001f;
	const float MAX_LENGTH = 4.0f / MATRIX_COUNT;
	if( animTime <= th ) {
		float a = clamp( (animTime-ts)/(th-ts) );
		dirLerp = a;
		wantedLength = a * MAX_LENGTH + MIN_LENGTH;
	} else {
		float a = clamp( 1 - (animTime-th)/(th-ts) );
		dirLerp = 1.0f;
		wantedLength = a * MAX_LENGTH + MIN_LENGTH;
	}

	// figure out wanted direction
	SVector3 wantedDir = handMat.getAxisZ() * (1-dirLerp) + handMat.getAxisX() * dirLerp;
	wantedDir.normalize();

	// construct bolt skinning matrices
	static const int MAT_INDEX_MAP[MATRIX_COUNT] = { 1, 2, 3, 0, }; // match animation!

	// base matrix
	SMatrix4x4 mbase;
	mbase.identify();
	mbase.getAxisZ() = -wantedDir;
	mbase.spaceFromAxisZ();

	for( int i = 0; i < MATRIX_COUNT; ++i ) {
		mats[i] = mbase;
		mats[i].getOrigin() = handMat.getOrigin() + wantedDir * (wantedLength * i);
	}
}


void CSceneInteractive::animateAttack1( time_value animTime )
{
	const CControllableCharacter::SWholeAttackParams& atkParams = mCharacter->getAttackParams( mAttackIndex );
	float animTimeS = animTime.tosec();
	const SMatrix4x4& mhandL = mCharacter->getAnimator().getBoneWorldMatrices()[mHandLIndex];
	const SMatrix4x4& mhandR = mCharacter->getAnimator().getBoneWorldMatrices()[mHandRIndex];
	animateAttack1Bolt( animTimeS, mhandL, mAttack1L->getAnimator().getBoneWorldMatrices(), atkParams.l.timeStart, atkParams.l.timeHit, atkParams.addY );
	animateAttack1Bolt( animTimeS, mhandR, mAttack1R->getAnimator().getBoneWorldMatrices(), atkParams.r.timeStart, atkParams.r.timeHit, atkParams.addY );
	mAttack1L->getSkinUpdater().update();
	mAttack1R->getSkinUpdater().update();
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

	// figure out current attack type, if any
	if( !mCharacter->getAnimator().isPlayingOneShotAnim() ) {
		mAttackType = -1;
		mAttackIndex = -1;
		mAttackAnimStartTime = time_value(-1);
	} else {
		if( mAttackIndex == 2 || mAttackIndex == 3 ) {
			mAttackType = 1;
			time_value animTime = demoTime - mAttackAnimStartTime;
			mAttack2_1->getWorldMatrix() = mCharacter->getWorldMatrix();
			mAttack2_2->getWorldMatrix() = mCharacter->getWorldMatrix();
			mAttack2_1->update( animTime );
			mAttack2_2->update( animTime );
		} else {
			mAttackType = 0;
			time_value animTime = demoTime - mAttackAnimStartTime;
			animateAttack1( animTime );
		}
	}


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
	gDOFParams.set( dofDist, 1.0f / dofRange, 0.0f, 0.0f );
	gSetDOFBlurBias( 0.0f );

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

	if( mAttackType == 1 ) {
		mAttack2_1->render( renderMode );
		mAttack2_2->render( renderMode );
	} else if( mAttackType == 0 ) {
		mAttack1L->render( renderMode );
		mAttack1R->render( renderMode );
	}
	
	int i, n;
	n = mRoom.size();
	for( i = 0; i < n; ++i ) {
		mRoom[i]->render( renderMode );
	}
}


void CSceneInteractive::processInput( float mov, float rot, bool attack, time_value demoTime, float dt )
{
	mCharacter->move( mov, demoTime, dt );
	mCharacter->rotate( rot, dt );
	if( attack && mAttackStartTime.value < 0 ) {
		mAttackIndex = mCharacter->attack( demoTime );
		mAttackStartTime = demoTime + time_value::fromsec(0.5f);
		mAttackAnimStartTime = demoTime;
	}
}

const SMatrix4x4* CSceneInteractive::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
