#include "stdafx.h"
#include "SceneScroller.h"
#include "ComplexStuffEntity.h"



// --------------------------------------------------------------------------

const float CSceneScroller::SCROLLER_DURATION = 41; // seconds



CSceneScroller::CSceneScroller()
:	mStartTime(0)
,	mPlayedTime(-1.0f)
,	mByeAnimDuration(0)
{
	mCharacter = new CComplexStuffEntity( "Bicas", "Idle_v01" );
	addAnimEntity( *mCharacter );
	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// preload anims
	mAnims.push_back( RGET_ANIM("ScrollerClock") );	mAnimPlayCount.push_back( 0 );
	mAnims.push_back( RGET_ANIM("ScrollerDance") );	mAnimPlayCount.push_back( 0 );
	mAnims.push_back( RGET_ANIM("ScrollerJumpOut") ); mAnimPlayCount.push_back( 0 );
	mAnims.push_back( RGET_ANIM("ScrollerTins") ); mAnimPlayCount.push_back( 0 );
	mAnims.push_back( RGET_ANIM("ScrollerViewer") ); mAnimPlayCount.push_back( 0 );
	mByeAnim = RGET_ANIM("ScrollerRevB");
	mByeAnimDuration = gGetAnimDuration( *mByeAnim, false );
	
	mCharacter->getAnimator().playDefaultAnim( time_value() );

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
	mLocalTime = demoTime - mStartTime;
	mPlayedTime = mLocalTime.tosec();

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
