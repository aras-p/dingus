// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "CharacterAnimator.h"

using namespace dingus;


CCharacterAnimator::CCharacterAnimator()
:	mHasScale3( false )
,	mDefaultBunch( NULL )
,	mDefaultDuration( 1.0f )
,	mDefaultFadeInTime( 0.0f )
,	mCurrAnim( NULL )
{
	mRootMatrix.identify();

	// create mixers for transition between animations
	mPosMixer = new TVector3Mixer();
	mRotMixer = new TQuatMixer();
	mScale3Mixer = new TVector3Mixer();
}


CCharacterAnimator::~CCharacterAnimator()
{
	delete mPosMixer;
	delete mRotMixer;
	delete mScale3Mixer;
}


void CCharacterAnimator::setNumCurves( int n )
{
	mBoneLocalPos.resize( n );
	mBoneLocalRot.resize( n );
	mBoneLocalScale.resize( n );
	mBoneLocal.resize( n );
	mBoneWorld.resize( n );
	mNumCurves = n;
}




void CCharacterAnimator::SAnimState::setupState( const CAnimationBunch* b, float startTime, float duration, float numCurves, bool& hasScale )
{
	bunch = b;
	if( b == NULL )
		return;
	CAnimationBunch::TVector3Animation* posAnim = b->findVector3Anim("pos");
	assert( posAnim );
	posStream = new TVectorAnimStream( *posAnim, duration, 0, numCurves, startTime );
	CAnimationBunch::TQuatAnimation* rotAnim = b->findQuatAnim("rot");
	assert( rotAnim );
	rotStream = new TQuatAnimStream( *rotAnim, duration, 0, numCurves, startTime );

	// scale3 optional
	CAnimationBunch::TVector3Animation* scale3Anim = b->findVector3Anim("scale3");
	if( scale3Anim ) {
		scale3Stream = new TVectorAnimStream( *scale3Anim, duration, 0, numCurves, startTime );
		hasScale = true;
	} else {
		scale3Stream = 0;
		hasScale = false;
	}
}



void CCharacterAnimator::playAnim( const CAnimationBunch& bunch, float duration, float fadeInTime, bool oneShot, double startTime )
{
	setNumCurves( bunch.getCurveCount() );

	// pop oldest state if full
	if( mAnims.full() )
		mAnims.pop_front();

	// setup current state to play single anim and push it
	SSynchAnimState as;
	as.anims[0].setupState( &bunch, startTime, duration, mNumCurves, mHasScale3 );
	as.anims[1].setupState( NULL, startTime, duration, mNumCurves, mHasScale3 );
	as.fadeIn = fadeInTime;
	as.oneShot = oneShot;
	mAnims.push_back( as );

	mCurrAnim = &bunch;
}


void CCharacterAnimator::playSynchAnims( const CAnimationBunch& bunch1, const CAnimationBunch& bunch2, float duration, float lerper, float fadeInTime, double startTime )
{
	assert( bunch1.getCurveCount() == bunch2.getCurveCount() );
	setNumCurves( bunch1.getCurveCount() );

	// push new dummy state if there was none
	if( mAnims.empty() )
		mAnims.push_back( SSynchAnimState() );

	SSynchAnimState* asCurr = &mAnims.back();
	bool wasPlayingSynch = (asCurr->anims[1].bunch != NULL);

	// if we were not playing synch, then push new state to get the transition
	if( !wasPlayingSynch ) {
		// pop oldest state if full
		if( mAnims.full() )
			mAnims.pop_front();

		mAnims.push_back( SSynchAnimState() );
		asCurr = &mAnims.back();
	}

	if( asCurr->anims[0].bunch == &bunch1 && asCurr->anims[1].bunch == &bunch2 )
	{
		// if both bunches are still the same, then only adjust the duration
		asCurr->anims[0].posStream->adjustDuration( duration );
		asCurr->anims[0].rotStream->adjustDuration( duration );
		asCurr->anims[1].posStream->adjustDuration( duration );
		asCurr->anims[1].rotStream->adjustDuration( duration );

	} else if( asCurr->anims[0].bunch == &bunch2 ) {
		// previously 1st anim now is 2nd
		// copy it to the to 2nd slot
		asCurr->anims[1] = asCurr->anims[0];
		asCurr->anims[1].posStream->adjustDuration( duration );
		asCurr->anims[1].rotStream->adjustDuration( duration );

		// setup 1st slot
		float t = anim_time();
		float relPlayTime = asCurr->anims[1].posStream->getRelTime( t );
		float startTime = t - relPlayTime * duration;
		asCurr->anims[0].setupState( &bunch1, startTime, duration, mNumCurves, mHasScale3 );

	} else if( asCurr->anims[1].bunch == &bunch1 ) {
		// previously 2nd anim now is 1st
		// copy it to the to 1st slot
		asCurr->anims[0] = asCurr->anims[1];
		asCurr->anims[0].posStream->adjustDuration( duration );
		asCurr->anims[0].rotStream->adjustDuration( duration );

		// setup 2nd slot
		float t = anim_time();
		float relPlayTime = asCurr->anims[0].posStream->getRelTime( t );
		float startTime = t - relPlayTime * duration;
		asCurr->anims[1].setupState( &bunch2, startTime, duration, mNumCurves, mHasScale3 );

	} else {
		// both anims are new, setup them
		if( wasPlayingSynch ) {
			float t = anim_time();
			float relPlayTime = asCurr->anims[0].posStream->getRelTime( t );
			startTime = t - relPlayTime * duration;
		}
		asCurr->anims[0].setupState( &bunch1, startTime, duration, mNumCurves, mHasScale3 );
		asCurr->anims[1].setupState( &bunch2, startTime, duration, mNumCurves, mHasScale3 );

	}

	asCurr->fadeIn = fadeInTime;
	asCurr->lerper = lerper;
	asCurr->oneShot = false;

	mCurrAnim = &bunch1;
}


void CCharacterAnimator::setDefaultAnim( const CAnimationBunch& anim, float duration, float fadeInTime )
{
	mDefaultBunch = &anim;
	mDefaultDuration = duration;
	mDefaultFadeInTime = fadeInTime;
}

void CCharacterAnimator::playDefaultAnim( double startTime )
{
	assert( mDefaultBunch );
	playAnim( *mDefaultBunch, mDefaultDuration, mDefaultFadeInTime, false, startTime );
}

bool CCharacterAnimator::isPlayingDefaultAnim() const
{
	return (!mAnims.empty()) && (mAnims.back().anims[0].bunch == mDefaultBunch);
}

bool CCharacterAnimator::isPlayingOneShotAnim() const
{
	return (!mAnims.empty()) && (mAnims.back().oneShot == true);
}


void CCharacterAnimator::updateLocal()
{
	// if no animation - bail out
	if( !mCurrAnim )
		return;


	//
	// animate

	mPosMixer->clearStreams();
	mRotMixer->clearStreams();
	mScale3Mixer->clearStreams();


	float curTime = anim_time();

	const SSynchAnimState* stateCurr = &mAnims.back();

	if( stateCurr->oneShot ) {
		float curPlayTime = curTime - stateCurr->anims[0].posStream->getStartTime();
		float curDuration = stateCurr->anims[0].posStream->getDuration();
		if( curPlayTime >= curDuration - mDefaultFadeInTime ) {
			playDefaultAnim();
			curPlayTime = 0.0f;
			stateCurr = &mAnims.back();
		}
	}

	float accumWeight = 1.0f;

	int nhistory = mAnims.size();
	int animIdx = nhistory-1;
	bool leaveJustCurr = false;
	do {
		const SSynchAnimState& ascurr = mAnims[animIdx];
		float baseWeight = accumWeight;
		if( animIdx > 0 ) {
			const SSynchAnimState& asprev = mAnims[animIdx-1];
			float curPlayTime = curTime - ascurr.anims[0].posStream->getStartTime();
			float fadeInTime = ascurr.fadeIn;
			float fadeLerp = curPlayTime / fadeInTime;
			if( fadeLerp >= 1.0f ) {
				fadeLerp = 1.0f;
				if( animIdx == nhistory-1 ) {
					leaveJustCurr = true;
					break;
				}
			}
			baseWeight *= fadeLerp;
			accumWeight *= (1.0f-fadeLerp);
		}

		if( ascurr.anims[0].bunch ) {
			float wht = (1.0f - ascurr.lerper) * baseWeight;
			mPosMixer->addStream( ascurr.anims[0].posStream, wht );
			mRotMixer->addStream( ascurr.anims[0].rotStream, wht );
			if( mHasScale3 )
				mScale3Mixer->addStream( ascurr.anims[0].scale3Stream, wht );
		}
		if( ascurr.anims[1].bunch ) {
			float wht = (ascurr.lerper) * baseWeight;
			mPosMixer->addStream( ascurr.anims[1].posStream, wht );
			mRotMixer->addStream( ascurr.anims[1].rotStream, wht );
			if( mHasScale3 )
				mScale3Mixer->addStream( ascurr.anims[1].scale3Stream, wht );
		}

		--animIdx;

	} while( animIdx >= 0 );


	if( leaveJustCurr ) {
		while( mAnims.size() > 1 )
			mAnims.pop_front();
	}

	// mix the streams
	mPosMixer->update( mNumCurves, &mBoneLocalPos[0], true );
	mRotMixer->update( mNumCurves, &mBoneLocalRot[0], true );
	if( mHasScale3 )
		mScale3Mixer->update( mNumCurves, &mBoneLocalScale[0], true );

	
	//
	// calculate bone local matrices

	int n = mNumCurves;
	for( int b = 0; b < n; ++b ) {
		mBoneLocal[b] = SMatrix4x4( mBoneLocalPos[b],mBoneLocalRot[b] );
		if( mHasScale3 ) {
			mBoneLocal[b].getAxisX() *= mBoneLocalScale[b].x;
			mBoneLocal[b].getAxisY() *= mBoneLocalScale[b].y;
			mBoneLocal[b].getAxisZ() *= mBoneLocalScale[b].z;
		}
	}
}


void CCharacterAnimator::updateWorld()
{
	// if no animation - bail out
	if( !mCurrAnim )
		return;

	//
	// calculate bone world matrices

	const CAnimationBunch& bunch = *mCurrAnim;
	int n = mNumCurves;
	for( int b = 0; b < n; ++b ) {
		// world space bone matrix (local*parent)
		int boneParent = bunch.getCurveParent( b );
		assert( (boneParent>=0 && boneParent<b) || (boneParent==-1) );

		if( boneParent >= 0 )
			mBoneWorld[b].fromMultiply( mBoneLocal[b], mBoneWorld[boneParent] );
		else
			mBoneWorld[b].fromMultiply( mBoneLocal[b], mRootMatrix );
	}
}
