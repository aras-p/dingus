#ifndef __CONROLLABLE_CHARACTER_H
#define __CONROLLABLE_CHARACTER_H


#include "ComplexStuffEntity.h"



class CControllableCharacter : public CComplexStuffEntity {
public:
	struct SAttackParams {
		float	timeStart;	// negative if none
		float	timeHit;
	};
	struct SWholeAttackParams {
		SWholeAttackParams( float tsL, float thL, float tsR, float thR, float ay );

		SAttackParams l, r;
		float	addY;
	};
public:
	CControllableCharacter( float minX, float minZ, float maxX, float maxZ );
	~CControllableCharacter();

	void	move( float accel, time_value timenow, float dt );
	void	rotate( float targetSpeed, float dt );
	int		attack( time_value timenow );

	const SWholeAttackParams& getAttackParams( int index ) const { return mAttackParams[index]; }
	
private:
	struct SAnimParams {
	public:
		SAnimParams( CAnimationBunch* anm, float moveAmnt );
	public:
		CAnimationBunch*	anim;
		float		duration;
		float		moveSpeed;	// for one second
		float		moveAmount; // for one walk cycle
	};
	typedef std::vector<SAnimParams>	TAnimParamsVector;

private:
	TAnimParamsVector	mAnimsIdle;
	TAnimParamsVector	mAnimsMove;
	TAnimParamsVector	mAnimsAttack;
	std::vector<SWholeAttackParams>	mAttackParams;

	float	mMinX, mMaxX;
	float	mMinZ, mMaxZ;

	float	mMoveVelocity;
	float	mRotateVelocity;

	float	mMoveAccel;
	float	mRotateAccel;
};


#endif
