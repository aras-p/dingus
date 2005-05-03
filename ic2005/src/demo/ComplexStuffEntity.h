#ifndef __COMPLEX_STUFF_ENTITY_H
#define __COMPLEX_STUFF_ENTITY_H

#include <dingus/renderer/RenderableSkin.h>
#include <dingus/gfx/skeleton/CharacterAnimator.h>
#include <dingus/gfx/skeleton/SkinUpdater.h>
#include "DemoResources.h"


// --------------------------------------------------------------------------

inline static double gGetAnimDuration( CAnimationBunch& b, bool loopLast )
{
	const double ANIM_FPS = 30.0f;
	CAnimationBunch::TVector3Animation* a = b.findVector3Anim("pos");
	assert( a );
	return (a->getLength() - (loopLast ? 0 : 1)) / ANIM_FPS;
}


// --------------------------------------------------------------------------

class CComplexStuffEntity : public boost::noncopyable {
public:
	CComplexStuffEntity( const char* name, const char* defaultAnim );
	~CComplexStuffEntity();

	void	render( eRenderMode renderMode );
	void	update();

	const SMatrix4x4& getWorldMatrix() const { return mAnimator->getRootMatrix(); }
	SMatrix4x4& getWorldMatrix() { return mAnimator->getRootMatrix(); }
	const CCharacterAnimator& getAnimator() const { return *mAnimator; }
	CCharacterAnimator& getAnimator() { return *mAnimator; }

protected:
	CRenderableSkin*		mMesh[RMCOUNT];
	CCharacterAnimator*		mAnimator;
	CSkinUpdater*			mSkinUpdater;
};



#endif
