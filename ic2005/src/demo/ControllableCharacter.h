#ifndef __CONROLLABLE_CHARACTER_H
#define __CONROLLABLE_CHARACTER_H


#include "ComplexStuffEntity.h"



class CControllableCharacter : public CComplexStuffEntity {
public:
	CControllableCharacter( float minX, float minZ, float maxX, float maxZ );
	~CControllableCharacter();

	void	move( float accel, time_value timenow );
	void	rotate( float targetSpeed );
	int		attack( time_value timenow );
	
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

	float	mMinX, mMaxX;
	float	mMinZ, mMaxZ;

	float	mMoveVelocity;
	float	mRotateVelocity;

	float	mMoveAccel;
	float	mRotateAccel;
};


#endif
