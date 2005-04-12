// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"
#include "SkinUpdater.h"

using namespace dingus;


CSkinUpdater::CSkinUpdater( const CSkeletonInfo& skelInfo, const CCharacterAnimator& animator )
:	mSkeletonInfo(&skelInfo), mAnimator( &animator )
{
	// create storage for 4x3 skin matrices
	mSkinMatrices = new SVector3[ skelInfo.getBoneCount() * 4 ];
	for( int i = 0; i < skelInfo.getBoneCount(); ++i ) {
		mSkinMatrices[i*4+0].set(1,0,0);
		mSkinMatrices[i*4+1].set(0,1,0);
		mSkinMatrices[i*4+2].set(0,0,1);
		mSkinMatrices[i*4+3].set(0,0,0);
	}
}

CSkinUpdater::~CSkinUpdater()
{
	delete[] mSkinMatrices;
}

void CSkinUpdater::update()
{
	// if no animation - bail out
	const CAnimationBunch* bunch = mAnimator->getCurrAnim();
	if( !bunch )
		return;

	// bone->curve mapping (first time only)
	if( mBoneToCurveMap.empty() ) {
		int n = mSkeletonInfo->getBoneCount();
		mBoneToCurveMap.resize(n);
		for( int b = 0; b < n; ++b ) {
			int curveIdx = bunch->getCurveIndexByName( mSkeletonInfo->getBone(b).getName() );
			assert( curveIdx >= 0 );
			mBoneToCurveMap[b] = curveIdx;
		}
	}

	// update skin matrices
	SMatrix4x4 skinMat;
	int n = mBoneToCurveMap.size();
	assert( n == mSkeletonInfo->getBoneCount() );
	for( int b = 0; b < n; ++b ) {
		// matrix for skinning = invPoseMatrix * boneWorldMatrix
		int curveIdx = mBoneToCurveMap[b];
		const SMatrix4x4& boneWorldMat = mAnimator->getBoneWorldMatrices()[curveIdx];
		skinMat.fromMultiply( mSkeletonInfo->getBone(b).getInvPoseMatrix(), boneWorldMat );
		SVector3* sm = &mSkinMatrices[b*4];
		sm[0] = skinMat.getAxisX();
		sm[1] = skinMat.getAxisY();
		sm[2] = skinMat.getAxisZ();
		sm[3] = skinMat.getOrigin();
	}
}
