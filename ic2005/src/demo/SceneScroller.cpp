#include "stdafx.h"
#include "SceneScroller.h"
#include "ComplexStuffEntity.h"
#include <time.h>
#include <dingus/gfx/gui/Gui.h>
#include <dingus/math/MathUtils.h>


// --------------------------------------------------------------------------

const float CSceneScroller::SCROLLER_DURATION = 60; // seconds

CSceneScroller::STextLine::STextLine( const char* t, int f, int& y )
:	text(t)
,	font(f)
,	ypos(y)
{
	switch( f ) {
	case 1:
		y += 36;
		color = 0xFF404040;
		break;
	case 2:
		y += 24;
		color = 0xFF606060;
		break;
	case 3:
		y += 20;
		color = 0xFF808080;
		break;
	case 4:
		y += 10;
		color = 0x00808080;
		break;
	}
}


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

	mCharacter = new CComplexStuffEntity( "Bicas", "Electricity", "Idle_v01", 0.5f );
	addAnimEntity( *mCharacter );
	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// load anims
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
	
	// scroller
	int sy = GUI_Y;
	mScroller.push_back( STextLine( "in.out.side: the shell", 1, sy ) );
	mScroller.push_back( STextLine( "A realtime demo for Imagine Cup 2005", 2, sy ) );
	mScroller.push_back( STextLine( "  by team 'nesnausk!'", 2, sy ) );
	mScroller.push_back( STextLine( NULL, 2, sy ) );
	mScroller.push_back( STextLine( "Authors:", 2, sy ) );
	mScroller.push_back( STextLine( "  Aras 'NeARAZ' Pranckevicius", 2, sy ) );
	mScroller.push_back( STextLine( "      programming, shaders, technology, UV maps", 3, sy ) );
	mScroller.push_back( STextLine( "  Paulius 'OneHalf' Liekis", 2, sy ) );
	mScroller.push_back( STextLine( "      concept, sketches, models, textures, effects", 3, sy ) );
	mScroller.push_back( STextLine( "  Antanas 'AB' Balvocius", 2, sy ) );
	mScroller.push_back( STextLine( "      animations, models, camera", 3, sy ) );
	mScroller.push_back( STextLine( "  Raimundas 'mookid' Juska", 2, sy ) );
	mScroller.push_back( STextLine( "      music and sound fx", 3, sy ) );
	mScroller.push_back( STextLine( NULL, 2, sy ) );
	mScroller.push_back( STextLine( "This demo features:", 2, sy ) );
	mScroller.push_back( STextLine( NULL, 4, sy ) );
	mScroller.push_back( STextLine( "Effects: normal mapping, ambient occlusion,", 3, sy ) );
	mScroller.push_back( STextLine( "soft shadows, glossy reflections, depth of field.", 3, sy ) );
	mScroller.push_back( STextLine( NULL, 4, sy ) );
	mScroller.push_back( STextLine( "Artwork: models, diffuse/gloss textures,", 3, sy ) );
	mScroller.push_back( STextLine( "normal/AO textures, character & camera", 3, sy ) );
	mScroller.push_back( STextLine( "animations, special effects, music & sound.", 3, sy ) );
	mScroller.push_back( STextLine( NULL, 4, sy ) );
	mScroller.push_back( STextLine( "Specials: physics, interactive mode.", 3, sy ) );
	mScroller.push_back( STextLine( NULL, 2, sy ) );
	mScroller.push_back( STextLine( "We really don't know the name of", 2, sy ) );
	mScroller.push_back( STextLine( "this guy on the right side. But you", 2, sy ) );
	mScroller.push_back( STextLine( "can be sure he's a good person!", 2, sy ) );
	mScroller.push_back( STextLine( NULL, 2, sy ) );
	mScroller.push_back( STextLine( "You may press Esc now or wait a bit to enter", 3, sy ) );
	mScroller.push_back( STextLine( "interactive mode. Yay!", 3, sy ) );
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
	gCharTimeBlend = 0.0f;
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


static const float BLUR_TIME = 30.0f / ANIM_FPS;
static const float WHITE_TIME = 15.0f / ANIM_FPS;


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

	// dof/blur
	const float FOCUS_DISTANCE = 4.5853f;

	float dofBias = 0.0f;
	dofBias = max( dofBias, clamp( 1-mPlayedTime/BLUR_TIME ) );
	dofBias = max( dofBias, clamp( (mPlayedTime-SCROLLER_DURATION+BLUR_TIME)/BLUR_TIME ) );

	float dofColor = 0.0f;
	dofColor = max( dofColor, clamp( 1-mPlayedTime/WHITE_TIME ) );
	dofColor = max( dofColor, clamp( (mPlayedTime-SCROLLER_DURATION+WHITE_TIME)/WHITE_TIME ) );
	dofColor *= dofColor; // quadratic

	gSetDOFBlurBias( dofBias );
	gDOFParams.set( FOCUS_DISTANCE, 1.0f/3.0f, dofBias*2, dofColor );


	// animations
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
	
	mCharacter->update( mLocalTime );
}


void CSceneScroller::render( eRenderMode renderMode )
{
	mCharacter->render( renderMode );
	mFloor->render( renderMode );
}


void CSceneScroller::renderUI( CUIDialog& dlg )
{
	SUIElement textElem;
	memset( &textElem, 0, sizeof(textElem) );
	textElem.fontIdx = 1;
	textElem.textFormat = DT_LEFT | DT_TOP | DT_NOCLIP;
	textElem.colorFont.current = 0xFF404040;

	SFRect textRC;
	textRC.left = 0.1f * GUI_X;
	textRC.right = 0.7f * GUI_X;
	textRC.top = 0.1f * GUI_Y;
	textRC.bottom = GUI_Y;

	float dy = mLocalTime.tosec()*20.0f;

	const float FADE_Y = GUI_Y*0.1f;

	int n = mScroller.size();
	for( int i = 0; i < n; ++i ) {
		const STextLine& tl = mScroller[i];
		if( tl.text == NULL )
			continue;
		textRC.top = tl.ypos - dy;
		if( textRC.top + 50 < 0 || textRC.top > GUI_Y )
			continue;

		textElem.colorFont.current = tl.color;
		if( textRC.top < FADE_Y )
			textElem.colorFont.current.a = textRC.top/FADE_Y;
		else if( textRC.top > GUI_Y-FADE_Y-30 )
			textElem.colorFont.current.a = 1 - (textRC.top-GUI_Y+FADE_Y+30)/FADE_Y;

		textElem.fontIdx = tl.font;


		dlg.drawText( tl.text, &textElem, &textRC, false );
	}

	textElem.textFormat = DT_RIGHT | DT_BOTTOM | DT_NOCLIP;
	textElem.colorFont.current.a = 0.5f;
	textRC.right = GUI_X - 5;
	textRC.bottom = GUI_Y - 5;
	textElem.fontIdx = 0;
	dlg.drawText( "press space to enter interactive mode", &textElem, &textRC, false );
}


const SMatrix4x4* CSceneScroller::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
