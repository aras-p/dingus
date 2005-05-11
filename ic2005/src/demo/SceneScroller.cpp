#include "stdafx.h"
#include "SceneScroller.h"
#include "ComplexStuffEntity.h"
#include <dingus/utils/Random.h>



// --------------------------------------------------------------------------

const float CSceneScroller::SCROLLER_DURATION = 41; // seconds
//const float CSceneScroller::SCROLLER_DURATION = 241; // seconds

//const float SCROLLER_ANIM_SPEEDUP = 0.9f;


static float gGetNextDefAnimPlayTime()
{
	return gRandom.getFloat( 2.0f, 5.0f );
}


CSceneScroller::CSceneScroller()
:	mStartTime(0)
,	mPlayedTime(-1.0f)
,	mByeAnimDuration(0)
,	mDefAnimPlayedTime(0.0f)
,	mDefAnimPlayTime( gGetNextDefAnimPlayTime() )
{
	mCharacter = new CComplexStuffEntity( "Bicas", "Idle_v01", 0.5f );
	addAnimEntity( *mCharacter );
	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// preload anims
	mAnims.push_back( RGET_ANIM("ScrollerClock") );
	mAnims.push_back( RGET_ANIM("ScrollerDance") );
	mAnims.push_back( RGET_ANIM("ScrollerJumpOut") );
	mAnims.push_back( RGET_ANIM("ScrollerTins") );
	mAnims.push_back( RGET_ANIM("ScrollerViewer") );
	mAnimPlayCount.resize( mAnims.size(), 0 );

	mByeAnim = RGET_ANIM("ScrollerRevB");
	mByeAnimDuration = gGetAnimDuration( *mByeAnim, false );
	
	// position camera
	SMatrix4x4 mr;
	D3DXMatrixRotationX( &mr, D3DX_PI/2 );
	getCamera().mWorldMat = mr * SMatrix4x4(
		SVector3( -4.51482f, 1.73471f, 0.123245f ),
		SQuaternion( -0.499555f, 0.501168f, 0.413188f, 0.573193f )
	);
	
	// TBD: DOF: focus distance = 4.5853
}


CSceneScroller::~CSceneScroller()
{
}


void CSceneScroller::start( time_value demoTime )
{
	mStartTime = demoTime;
	mPlayedTime = 0.0f;
	mLocalTime = time_value(0);
	mCharacter->getAnimator().playDefaultAnim( mLocalTime );
}


void CSceneScroller::startScrollerAnim()
{
	CConsole::CON_WARNING << "Start scroller anim" << endl;

	mDefAnimPlayTime = gGetNextDefAnimPlayTime();
	mDefAnimPlayedTime = 0.0f;

	// we don't want to start anims longer than this
	float availableTime = SCROLLER_DURATION - mByeAnimDuration - mPlayedTime;

	// half-randomly select scroller animation
	int bestPlayCount = 100;
	int bestAnimIdx = -1;

	int n = mAnims.size();
	for( int i = 0; i < n; ++i ) {
		int idx = gRandom.getUInt() % n;
		const CAnimationBunch* anim = mAnims[idx];
		if( gGetAnimDuration(*anim,false) < availableTime ) {
			if( mAnimPlayCount[idx] < bestPlayCount ) {
				bestPlayCount = mAnimPlayCount[idx];
				bestAnimIdx = idx;
			}
		}
	}

	// if we have animation, play it
	if( bestAnimIdx >= 0 ) {
		CConsole::CON_WARNING << "Found one, play!" << endl;

		++mAnimPlayCount[bestAnimIdx];
		const CAnimationBunch* anim = mAnims[bestAnimIdx];
		mCharacter->getAnimator().playAnim( *anim, gGetAnimDuration(*anim,false), 0.5f, true, mLocalTime );
	}
}


void CSceneScroller::update( time_value demoTime, float dt )
{
	// update camera projection
	const float CAMERA_FOV = D3DXToRadian( 41.1121f );
	const float camnear = 0.1f;
	const float camfar = 50.0f;
	float aspect = CD3DDevice::getInstance().getBackBufferAspect();
	getCamera().setProjectionParams( CAMERA_FOV / aspect, aspect, camnear, camfar );

	// timing
	time_value oldLocalTime = mLocalTime;
	mLocalTime = demoTime - mStartTime;
	mPlayedTime = mLocalTime.tosec();
	float dtFromTimer = (mLocalTime-oldLocalTime).tosec();


	if( mPlayedTime >= SCROLLER_DURATION - mByeAnimDuration + 2.0f ) {
		// if we're ending, play bye anim
		if( mCharacter->getAnimator().getCurrAnim() != mByeAnim ) {
			mCharacter->getAnimator().playAnim( *mByeAnim, gGetAnimDuration(*mByeAnim,false), 0.8f, true, mLocalTime );
		}

	} else {

		// track how long we're playing idle anim
		if( mCharacter->getAnimator().isPlayingDefaultAnim() ) {
			mDefAnimPlayedTime += dtFromTimer;

			// if too long - start scroller anim
			if( mDefAnimPlayedTime >= mDefAnimPlayTime ) {
				startScrollerAnim();
			}
		}
	}
	

	// TBD: select and play scroller anims!

	mCharacter->update( mLocalTime );
}


void CSceneScroller::render( eRenderMode renderMode )
{
	mCharacter->render( renderMode );
}

const SMatrix4x4* CSceneScroller::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
