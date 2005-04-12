// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __CHAR_ANIMATOR_H
#define __CHAR_ANIMATOR_H

#include "../../math/Matrix4x4.h"
#include "../../animator/AnimationBunch.h"
#include "../../animator/AnimStream.h"
#include "../../animator/AnimImmediateMixer.h"
#include "../../utils/ringdeque.h"

namespace dingus {



class CCharacterAnimator : public boost::noncopyable {
public:
	CCharacterAnimator();
	~CCharacterAnimator();


	void	playSynchAnims( const CAnimationBunch& bunch1, const CAnimationBunch& bunch2, float duration, float lerper, float fadeInTime );

	void	playAnim( const CAnimationBunch& bunch, float duration, float fadeInTime, bool oneShot );

	void	playDefaultAnim();

	bool	isPlayingDefaultAnim() const;
	bool	isPlayingOneShotAnim() const;

	/**
	 *	Adds 'default' animation. This is played when there's nothing
	 *  else to play.
	 */
	void	setDefaultAnim( const CAnimationBunch& anim, float duration, float fadeInTime );



	/**
	 *  Evaluates animations, computes local transforms.
	 */
	void	updateLocal();
	/**
	 *  Computes world transforms from current local transforms.
	 */
	void	updateWorld();

	
	/**
	 *	Returns 'current' animation. NOTE: use it only for curve count/hierarchy,
	 *  as 'current' animations can be anything when there are multiple
	 *  animations playing.
	 */
	const CAnimationBunch* getCurrAnim() const { return mCurrAnim; }

	const SMatrix4x4& getRootMatrix() const { return mRootMatrix; }
	SMatrix4x4& getRootMatrix() { return mRootMatrix; }

	const SMatrix4x4* getBoneLocalMatrices() const { return &mBoneLocal[0]; }
	SMatrix4x4* getBoneLocalMatrices() { return &mBoneLocal[0]; }
	const SMatrix4x4* getBoneWorldMatrices() const { return &mBoneWorld[0]; }
	SMatrix4x4* getBoneWorldMatrices() { return &mBoneWorld[0]; }

protected:
	void	setNumCurves( int n );

private:
	typedef CAnimImmediateMixer<TVectorAnimStream>	TVector3Mixer;
	typedef CAnimImmediateMixer<TQuatAnimStream>	TQuatMixer;
	typedef std::vector<SVector3>		TVec3Vector;
	typedef std::vector<SQuaternion>	TQuatVector;
	typedef std::vector<SMatrix4x4>		TMatrixVector;

	struct SAnimState {
		SAnimState() : bunch(0), posStream(0), rotStream(0)/*, scale3Stream(0), fadeIn(0)*/ { }

		void	setupState( const CAnimationBunch* b, float startTime, float duration, float numCurves );
		void	resetState() { bunch = NULL; posStream=0; rotStream=0; /*scale3Stream=0;*/ }

		const CAnimationBunch*			bunch;
		TVectorAnimStream::TSharedPtr	posStream;
		TQuatAnimStream::TSharedPtr		rotStream;
		//TVectorAnimStream::TSharedPtr	scale3Stream;
	};
	struct SSynchAnimState {
		enum { ANIMS = 2 };

		SSynchAnimState() : lerper(0.0f), fadeIn(0.0f), weight(0.0f), oneShot(false) { }

		SAnimState	anims[ANIMS];
		float		lerper;
		float		fadeIn;
		float		weight;
		bool		oneShot;
	};

	typedef ringdeque<SSynchAnimState,4>		TAnimHistoryDeque;

private:
	/// Root matrix
	SMatrix4x4	mRootMatrix;

	TAnimHistoryDeque	mAnims;

	const CAnimationBunch*	mDefaultBunch;
	float			mDefaultDuration;
	float			mDefaultFadeInTime;


	/// Position mixer
	TVector3Mixer*	mPosMixer;
	/// Rotation mixer
	TQuatMixer*		mRotMixer;
	/// Scale3 mixer
	//TVector3Mixer*	mScale3Mixer;
	
	/// Current number of anim curves
	int				mNumCurves;

	/// Has scale3 animation? (if present, must be in all animations)
	//bool			mHasScale3;

	TVec3Vector		mBoneLocalPos;
	TQuatVector		mBoneLocalRot;
	//TVec3Vector		mBoneLocalScale;
	TMatrixVector	mBoneLocal;
	TMatrixVector	mBoneWorld;

	/**
	 *	'Current' animation. Used only for curve count and hierarchy,
	 *  so can be just the first animation if the others have the same
	 *  curve layout.
	 */
	const CAnimationBunch*	mCurrAnim;
};


}; // namespace

#endif
