#ifndef __SCENE_SCROLLER_H
#define __SCENE_SCROLLER_H

#include "Scene.h"
#include <dingus/utils/Random.h>


// --------------------------------------------------------------------------

class CSceneScroller : public CScene {
public:
	CSceneScroller();
	~CSceneScroller();

	virtual void	update( time_value demoTime, float dt );
	virtual void	render( eRenderMode renderMode );
	virtual void	renderUI( CUIDialog& dlg );
	virtual const SMatrix4x4* getLightTargetMatrix() const;
	
	virtual bool	needsReflections() const { return false; }

	void	start( time_value demoTime );
	bool	isEnded() const { return mPlayedTime >= SCROLLER_DURATION; }

private:
	static const float SCROLLER_DURATION;
	void	startScrollerAnim();

private:
	// random generator
	CRandomFast	mRandom;

	// timing
	time_value	mStartTime;
	time_value	mLocalTime; // time since started
	float		mPlayedTime;
	float		mByeAnimDuration;

	float		mDefAnimPlayedTime;
	float		mDefAnimPlayTime;

	// anims
	std::vector<CAnimationBunch*>	mAnims;
	std::vector<int>				mAnimPlayCount;
	CAnimationBunch*				mByeAnim;

	// character
	CComplexStuffEntity*	mCharacter;
	int			mSpineBoneIndex;

	// floor
	CMeshEntity*	mFloor;
};



#endif
