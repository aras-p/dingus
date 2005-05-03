#include "stdafx.h"
#include "ControllableCharacter.h"
#include <dingus/math/MathUtils.h>
#include <dingus/utils/Random.h>


static const float MOVE_FWD_CURVE_DATA[] = {
	0.0f,
	1.0f,
	1.1f,
	1.2f,
	1.2f,
	1.2f,
	1.2f,
	1.4f,
	1.7f,
	1.7f,
	1.7f,
	1.7f,
	1.9f,
	2.1f,
	2.5f,
};

static const float MOVE_BCK_CURVE_DATA[] = {
	0.0f,
	-0.30f,
	-0.50f,
	-0.60f,
	-0.60f,
	-0.60f,
	-0.64f,
	-0.75f,
};

static	CTabularFunction<float>	gMoveFwdCurve( sizeof(MOVE_FWD_CURVE_DATA)/sizeof(MOVE_FWD_CURVE_DATA[0]), MOVE_FWD_CURVE_DATA );
static	CTabularFunction<float>	gMoveBckCurve( sizeof(MOVE_BCK_CURVE_DATA)/sizeof(MOVE_BCK_CURVE_DATA[0]), MOVE_BCK_CURVE_DATA );



CControllableCharacter::SAnimParams::SAnimParams( CAnimationBunch* anm, float movAmnt )
:	anim(anm)
,	duration( gGetAnimDuration(*anm, true) )
,	moveAmount( movAmnt )
{
	moveSpeed = moveAmount / duration;
}



CControllableCharacter::CControllableCharacter( float minX, float minZ, float maxX, float maxZ )
:	CComplexStuffEntity( "Bicas", "Idle_v01" )
,	mMinX(minX), mMaxX(maxX), mMinZ(minZ), mMaxZ(maxZ)
,	mMoveVelocity( 0.0f )
,	mRotateVelocity( 0.0f )
,	mMoveAccel( 0.0f )
,	mRotateAccel( 0.0f )
{
	// idles
	mAnimsIdle.push_back( SAnimParams( RGET_ANIM("Idle_v01"), 0.0f ) );
	
	// walk animations, in increasing speed
	//mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Slow01"), 0.5856f ) );	// 1.20525 m/s
	////mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Slow01"), 1.2856f ) );	// 1.20525 m/s
	//mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Slow02"), 1.2856f ) );	// 1.20525 m/s
	//mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Fast"), 1.857f ) );		// 1.74094 m/s
	
	mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Back01"), -1.3381f ) );		// -0.637 m/s
	mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Back01"), -0.3f ) );
	mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Slow01"), 0.4679f ) );	// 1.20525 m/s
	mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Slow02"), 1.2466f ) );	// 1.20525 m/s
	mAnimsMove.push_back( SAnimParams( RGET_ANIM("LoopWalk_Fast"), 1.801f ) );		// 1.74094 m/s
	

	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v01"), 0 ) );
	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v01_2x"), 0 ) );
	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v02"), 0 ) );
	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v03"), 0 ) );
	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v03b"), 0 ) );
	mAnimsAttack.push_back( SAnimParams( RGET_ANIM("Attack_v03c"), 0 ) );

	getAnimator().setDefaultAnim( *mAnimsIdle[0].anim, mAnimsIdle[0].duration, 0.5f );
	getAnimator().playDefaultAnim( CSystemTimer::getInstance().getTime() );
}


CControllableCharacter::~CControllableCharacter()
{
}


/*
notes:

	We want to move at velocity V. That is in between our two anims, anim1 and
	anim2 (whose velocities are V1 and V2; V1 < V < V2; and natural durations
	are D1 and D2).

	Our lerper is: t = (V-V1) / (V2-V1);

	Our duration is lerped: D = D1 + t * (D2-D1);

*/

void	CControllableCharacter::move( float accel, time_value timenow )
{
	if( getAnimator().isPlayingOneShotAnim() )
		return;

	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	if( accel == 0.0f || accel*mMoveVelocity < 0.0f )
		mMoveVelocity = smoothCD( mMoveVelocity, 0.0f, mMoveAccel, 0.2f, dt );
	mMoveVelocity += accel * dt;
	mMoveVelocity = clamp( mMoveVelocity, -1.0f, 1.0f );

	float realVelocity;
	if( mMoveVelocity >= 0.0f )
		gMoveFwdCurve.eval( mMoveVelocity, realVelocity );
	else
		gMoveBckCurve.eval( -mMoveVelocity, realVelocity );

	// bound to the room
	SVector3 oldO = getWorldMatrix().getOrigin();
	SVector3 deltaO = -getWorldMatrix().getAxisX() * (realVelocity * dt);
	deltaO.y = 0.0f;
	float origDeltaLen = deltaO.length();

	/*
	const float BOUNDARY = 0.3f;
	if( oldO.x + deltaO.x > mMaxX - BOUNDARY ) {
		float t = (oldO.x + deltaO.x - (mMaxX-BOUNDARY)) / BOUNDARY;
		deltaO.x *= 1.0f - t;
	}
	*/

	if( oldO.x + deltaO.x > mMaxX )
		deltaO.x = mMaxX - oldO.x;
	else if( oldO.x + deltaO.x < mMinX )
		deltaO.x = mMinX - oldO.x;

	if( oldO.z + deltaO.z > mMaxZ )
		deltaO.z = mMaxZ - oldO.z;
	else if( oldO.z + deltaO.z < mMinZ )
		deltaO.z = mMinZ - oldO.z;

	float newDeltaLen = deltaO.length();
	if( newDeltaLen != origDeltaLen ) {
		float factor = newDeltaLen / origDeltaLen;
		realVelocity *= factor;
		//mMoveVelocity *= factor;
	}


	// find the anims that are between this velocity
	int anim1Idx = -1;
	if( realVelocity > 0.0f ) {
		for( int i = 0; i < mAnimsMove.size(); ++i ) {
			float animSpd = mAnimsMove[i].moveSpeed;
			if( animSpd > 0.0f && animSpd <= realVelocity ) {
				anim1Idx = i;
			}
		}
	} else if( realVelocity < 0.0f ) {
		for( int i = mAnimsMove.size()-1; i >= 0; --i ) {
			float animSpd = mAnimsMove[i].moveSpeed;
			if( animSpd < 0.0f && animSpd >= realVelocity ) {
				anim1Idx = i;
			}
		}

	}
	int anim2Idx = anim1Idx + (realVelocity>=0.0f ? 1 : -1);
	if( anim2Idx < 0 || anim2Idx >= mAnimsMove.size() || mAnimsMove[anim2Idx].moveSpeed * realVelocity < 0.0f )
		anim2Idx = anim1Idx;

	SAnimParams *anim1, *anim2;

	if( anim1Idx < 0 || anim2Idx < 0 ) {

		if( !getAnimator().isPlayingDefaultAnim() )
			getAnimator().playDefaultAnim( timenow );

	} else {

		getWorldMatrix().getOrigin() = oldO + deltaO;

		float lerper, duration;

		assert( anim1Idx >=0 && anim1Idx < mAnimsMove.size() );
		assert( anim2Idx >=0 && anim2Idx < mAnimsMove.size() );
		anim1 = &mAnimsMove[anim1Idx];
		anim2 = &mAnimsMove[anim2Idx];
		float speedDiff = anim2->moveSpeed - anim1->moveSpeed;
		if( fabsf(speedDiff) > 1.0e-3f )
			lerper = (realVelocity - anim1->moveSpeed) / speedDiff;
		else
			lerper = 0.0f;
		duration = anim1->duration + lerper * (anim2->duration - anim1->duration);

		getAnimator().playSynchAnims( *anim1->anim, *anim2->anim, duration, lerper, 0.3f, timenow, timenow );
	}
}

void	CControllableCharacter::rotate( float targetSpeed )
{
	if( getAnimator().isPlayingOneShotAnim() )
		targetSpeed *= 0.25f;

	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	if( targetSpeed == 0.0f )
		mRotateVelocity = smoothCD( mRotateVelocity, 0.0f, mRotateAccel, 0.1f, dt );
	else
		mRotateVelocity = smoothCD( mRotateVelocity, targetSpeed, mRotateAccel, 0.1f, dt );
	//mMoveVelocity = clamp( mMoveVelocity, -1.0f, 1.0f );

	SMatrix4x4 mr;
	D3DXMatrixRotationY( &mr, mRotateVelocity * dt );
	getWorldMatrix() = mr * getWorldMatrix();
}

void CControllableCharacter::attack( time_value timenow )
{
	if( getAnimator().isPlayingOneShotAnim() )
		return;

	int idx = gRandom.getInt() % mAnimsAttack.size();
	getAnimator().playAnim( *mAnimsAttack[idx].anim, mAnimsAttack[idx].duration, 0.2f, true, timenow );

	mMoveVelocity = mMoveAccel = 0.0f;
}
