#include "stdafx.h"
#include "IdlePlayer.h"
#include "ComplexStuffEntity.h"
#include <dingus/utils/Random.h>

// --------------------------------------------------------------------------

void InitIdleConfig( IdleConfig& cfg )
{
	cfg.mAnims.push_back( RGET_ANIM("ScrollerClock") );
	cfg.mAnims.push_back( RGET_ANIM("ScrollerDance") );
	cfg.mAnims.push_back( RGET_ANIM("ScrollerFly") );
	cfg.mAnims.push_back( RGET_ANIM("ScrollerJumpOut") );
	cfg.mAnims.push_back( RGET_ANIM("ScrollerTins") );
	cfg.mAnims.push_back( RGET_ANIM("ScrollerViewer") );
}

// --------------------------------------------------------------------------


static float gGetNextDefAnimPlayTime()
{
	return gRandom.getFloat( 2.0f, 8.0f );
}


IdlePlayer::IdlePlayer( const IdleConfig& cfg )
:	mCfg( &cfg )
,	mStartTime(0)
,	mLocalTime(0)
,	mDefAnimPlayedTime(0.0f)
,	mDefAnimPlayTime( gGetNextDefAnimPlayTime() )
{
	mAnimPlayCount.resize( cfg.mAnims.size(), 0 );
}


IdlePlayer::~IdlePlayer()
{
}


void IdlePlayer::startScrollerAnim( CComplexStuffEntity& character )
{
	mDefAnimPlayTime = gGetNextDefAnimPlayTime();
	mDefAnimPlayedTime = 0.0f;

	// half-randomly select animation
	int n = mCfg->mAnims.size();
	int bestAnimIdx = gRandom.getUInt() % n;
	int bestPlayCount = mAnimPlayCount[bestAnimIdx];
	for( int i = 0; i < n / 2; ++i ) {
		int idx = gRandom.getUInt() % n;
		const CAnimationBunch* anim = mCfg->mAnims[idx];
		if( mAnimPlayCount[idx] < bestPlayCount ) {
			bestPlayCount = mAnimPlayCount[idx];
			bestAnimIdx = idx;
		}
	}

	++mAnimPlayCount[bestAnimIdx];
	const CAnimationBunch* anim = mCfg->mAnims[bestAnimIdx];
	character.getAnimator().playAnim( *anim, gGetAnimDuration(*anim,false), 0.5f, true, mLocalTime );
}


void IdlePlayer::update( CComplexStuffEntity& character, time_value demoTime )
{
	// timing
	time_value oldLocalTime = mLocalTime;
	mLocalTime = demoTime - mStartTime;
	float dtFromTimer = (mLocalTime-oldLocalTime).tosec();

	// track how long we're playing idle anim
	if( character.getAnimator().isPlayingDefaultAnim() ) {
		mDefAnimPlayedTime += dtFromTimer;

		// if too long - start scroller anim
		if( mDefAnimPlayedTime >= mDefAnimPlayTime ) {
			startScrollerAnim( character );
		}
	}
	
	character.update( mLocalTime );
}
