#ifndef __IDLE_PLAYER_H
#define __IDLE_PLAYER_H

class CComplexStuffEntity;


struct IdleConfig {
	std::vector<CAnimationBunch*>	mAnims;
};

void InitIdleConfig( IdleConfig& cfg );

// --------------------------------------------------------------------------

class IdlePlayer {
public:
	IdlePlayer( const IdleConfig& cfg );
	~IdlePlayer();

	void	update( CComplexStuffEntity& character, time_value demoTime );

private:
	void	startScrollerAnim( CComplexStuffEntity& character );

private:
	std::vector<int>	mAnimPlayCount;
	const IdleConfig*	mCfg;

	// timing
	time_value	mStartTime;
	time_value	mLocalTime; // time since started

	float		mDefAnimPlayedTime;
	float		mDefAnimPlayTime;
};



#endif
