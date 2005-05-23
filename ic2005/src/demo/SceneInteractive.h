#ifndef __SCENE_INTERACTIVE_H
#define __SCENE_INTERACTIVE_H

#include "Scene.h"


class CControllableCharacter;
class CThirdPersonCameraController;
class CSceneSharedStuff;


// --------------------------------------------------------------------------

class CSceneInteractive : public CScene {
public:
	CSceneInteractive( CSceneSharedStuff* sharedStuff );
	~CSceneInteractive();

	virtual void	update( time_value demoTime, float dt );
	virtual void	render( eRenderMode renderMode );
	virtual const SMatrix4x4* getLightTargetMatrix() const;
	
	void	start( time_value demoTime, CUIDialog& dlg );

	void	processInput( float mov, float rot, bool attack, time_value demoTime, float dt );

private:
	void	animateAttack1( time_value animTime );

private:
	CSceneSharedStuff*	mSharedStuff;
	
	CControllableCharacter*			mCharacter;
	int			mSpineBoneIndex;
	CThirdPersonCameraController*	mCamController;
	
	// outside room
	std::vector<CRoomObjectEntity*>	mRoom;

	// attacks related
	int				mAttackIndex;
	int				mAttackType;
	time_value		mAttackStartTime;	// negative if no attack
	time_value		mAttackAnimStartTime;
	time_value		mWallHitTime;		// negative if no attack
	SVector3		mWallHitPos;
	float			mWallHitRadius;

	// 1st attack type
	CComplexStuffEntity*	mAttack1L;
	CComplexStuffEntity*	mAttack1R;
	int			mHandLIndex;
	int			mHandRIndex;
	// hand world matrices in canonical animation
	SMatrix4x4		mMatLBegin, mMatLMid;
	SMatrix4x4		mMatRBegin, mMatRMid;
	// inverse hand world matrices in canonical animation
	// these * curr_anim_hands = M
	// curr_bone_mats = canon_bone_mats * M
	SMatrix4x4		mInvMatLBegin, mInvMatLMid;
	SMatrix4x4		mInvMatRBegin, mInvMatRMid;



	// 2nd attack type
	CComplexStuffEntity*	mAttack2_1;
	CComplexStuffEntity*	mAttack2_2;
};


#endif
