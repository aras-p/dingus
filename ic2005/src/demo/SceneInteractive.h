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
	void	animateAttack1Bolt( float animTime,
		const SMatrix4x4& handMat, SMatrix4x4* mats, float ts, float th, const SVector3& target );
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
	float		mTimeSource;

	// 2nd attack type
	CComplexStuffEntity*	mAttack2_1;
	CComplexStuffEntity*	mAttack2_2;
};


#endif
