// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __SKIN_UPDATER_H
#define __SKIN_UPDATER_H

#include "SkeletonInfo.h"
#include "CharacterAnimator.h"

namespace dingus {



class CSkinUpdater : public boost::noncopyable {
public:
	CSkinUpdater( const CSkeletonInfo& skelInfo, const CCharacterAnimator& animator );
	~CSkinUpdater();

	void	update();
	
	/**
	 *  Pointer to 4x3 skin matrices. Each matrix is four SVector3's (X,Y,Z,O).
	 *  This is intended for direct set-up to effect parameters (just set up
	 *  whole array).
	 */
	const SVector3* getSkinMatrices() const { return mSkinMatrices; }

private:
	typedef std::vector<int>			TIntVector;
private:
	/// The skeleton info
	const CSkeletonInfo*		mSkeletonInfo;

	/// The animator
	const CCharacterAnimator*	mAnimator;
	
	/// Skeleton bone indices for each anim curve
	TIntVector		mBoneToCurveMap;
	
	/// Skin 4x3 matrices (each is 4 SVector3's - X,Y,Z,O).
	SVector3*		mSkinMatrices;
};


}; // namespace

#endif
