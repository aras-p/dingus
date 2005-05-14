#include "stdafx.h"
#include "SceneScroller.h"
#include "ComplexStuffEntity.h"
#include <time.h>
#include <dingus/gfx/gui/Gui.h>



// --------------------------------------------------------------------------

//const float CSceneScroller::SCROLLER_DURATION = 41; // seconds
const float CSceneScroller::SCROLLER_DURATION = 61; // seconds



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
	mRandom.seed( time(NULL) );

	mCharacter = new CComplexStuffEntity( "Bicas", "Idle_v01", 0.5f );
	addAnimEntity( *mCharacter );
	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// preload anims
	mAnims.push_back( RGET_ANIM("ScrollerClock") );
	mAnims.push_back( RGET_ANIM("ScrollerDance") );
	mAnims.push_back( RGET_ANIM("ScrollerFly") );
	mAnims.push_back( RGET_ANIM("ScrollerJumpOut") );
	mAnims.push_back( RGET_ANIM("ScrollerTins") );
	mAnims.push_back( RGET_ANIM("ScrollerViewer") );
	mAnimPlayCount.resize( mAnims.size(), 0 );

	mByeAnim = RGET_ANIM("ScrollerRevB");
	mByeAnimDuration = gGetAnimDuration( *mByeAnim, false );

	// floor
	mFloor = new CMeshEntity( "ScrollerFloor", "billboard" );
	addEntity( *mFloor );
	
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
	mDefAnimPlayTime = gGetNextDefAnimPlayTime();
	mDefAnimPlayedTime = 0.0f;

	// we don't want to start anims longer than this
	float availableTime = SCROLLER_DURATION - mByeAnimDuration - mPlayedTime;

	// half-randomly select scroller animation
	int bestPlayCount = 100;
	int bestAnimIdx = -1;

	int n = mAnims.size();
	for( int i = 0; i < n; ++i ) {
		int idx = mRandom.getByte() % n;
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
	mFloor->render( renderMode );
}


void CSceneScroller::renderUI( CUIDialog& dlg )
{
	static const char* scrText =
		"Bah blah! Teh scroller!\n"
		"\n"
		"Hey, it's me, the scroller!\n"
		"Aaa!\n"
		"\n"
		"Here I am!\n"
		;

	SUIElement textElem;
	memset( &textElem, 0, sizeof(textElem) );
	textElem.fontIdx = 1;
	textElem.textFormat = DT_LEFT;
	textElem.colorFont.current = 0xFF404040;
	
	SFRect textRC;
	textRC.left = 0.1f * GUI_X;
	textRC.right = 0.5f * GUI_X;
	textRC.top = 0.1f * GUI_Y;
	textRC.bottom = 0.9f * GUI_Y;

	dlg.drawText( scrText, &textElem, &textRC, false );
}


const SMatrix4x4* CSceneScroller::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
