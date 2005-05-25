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
	virtual void	renderUI( CUIDialog& dlg );
	virtual const SMatrix4x4* getLightTargetMatrix() const;
	
	void	start( time_value demoTime, CUIDialog& dlg );

	void	processInput( float mov, float rot, bool attack, time_value demoTime, float dt );

private:
	void	animateAttack1Bolt( float demoTimeS, float animTime,
		const SMatrix4x4& handMat, SMatrix4x4* mats, float ts, float th, const SVector3& target );
	void	animateAttack1( float demoTimeS, time_value animTime );
	void	animateAttack2( float demoTimeS, time_value animTime );
	bool	calcAttackTargetPos( SVector3& pos, float addY, float range );

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
	time_value		mAttackAnimStartTime;
	SVector3		mWallHitPosL, mWallHitPosR;
	float			mWallHitRadius;
	int				mFracIntervalCounter;

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
