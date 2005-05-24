#ifndef __SCENE_MAIN_H
#define __SCENE_MAIN_H

#include "Scene.h"

class CSceneSharedStuff;
namespace dingus {
	class CCharacterAnimator;
};


// --------------------------------------------------------------------------


class CSceneMain : public CScene {
public:
	static const float EXTRA_FRAMES;
public:
	CSceneMain( CSceneSharedStuff* sharedStuff );
	~CSceneMain();

	virtual void	update( time_value demoTime, float dt );
	virtual void	render( eRenderMode renderMode );
	virtual void	renderUI( CUIDialog& dlg );
	virtual const SMatrix4x4* getLightTargetMatrix() const;
	bool	isEnded() const { return mCurrAnimFrame >= mAnimFrameCount + EXTRA_FRAMES; }

private:
	void	animateCamera();
	void	animateLocally( CComplexStuffEntity& e, float beginFrame );
	void	animateLocally( CComplexStuffEntity& e, float beginFrame, float endFrame );

private:
	CSceneSharedStuff*	mSharedStuff;

	// camera/DOF anim
	CAnimationBunch*	mCameraAnim;
	CAnimationBunch::TVector3Animation*	mCameraAnimPos;
	CAnimationBunch::TQuatAnimation*	mCameraAnimRot;
	CAnimationBunch::TVector3Animation*	mCameraAnimParams;
	CAnimationBunch*	mDOFAnim;
	CAnimationBunch::TVector3Animation*	mDOFAnimPos;
	CAnimationBunch::TFloatAnimation*	mDOFAnimRange;

	// timing
	double	mAnimFrameCount;
	double	mAnimDuration;
	double	mCurrAnimFrame;
	double	mCurrAnimAlpha;

	// main character
	CComplexStuffEntity*	mCharacter;
	int			mSpineBoneIndex;

	// other characters
	CComplexStuffEntity*	mCharacter2;
	CComplexStuffEntity*	mCharacter3;

	// for animating doors
	CCharacterAnimator*	mDoorsAnim;
	std::vector<int>	mDoorAnim2RoomIdx;

	// bed/stone
	CMeshEntity*			mBedStatic;
	CComplexStuffEntity*	mBedAnim;
	CComplexStuffEntity*	mStone;

	// attacks
	CComplexStuffEntity*	mAttack1;
	CComplexStuffEntity*	mAttack2_1;
	CComplexStuffEntity*	mAttack2_2;
	CComplexStuffEntity*	mAttack3;

	// room2
	CMeshEntity*			mRoom2Top;
	CMeshEntity*			mRoom2Bottom;

	// outside rooms
	std::vector<CRoomObjectEntity*>	mRoom;
	std::vector<CRoomObjectEntity*>	mRoom2;
	bool					mWasLight1;
};




#endif
