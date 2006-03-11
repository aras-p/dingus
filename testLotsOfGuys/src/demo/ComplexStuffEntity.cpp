#include "stdafx.h"
#include "ComplexStuffEntity.h"


// --------------------------------------------------------------------------


CComplexStuffEntity::CComplexStuffEntity( const char* name, const char* defaultAnim, float defAnimFadeInTime )
{
	mAnimator = new CCharacterAnimator();
	CAnimationBunch& anim = *RGET_ANIM(defaultAnim);
	float animDuration = gGetAnimDuration(anim,false);
	mAnimator->setDefaultAnim( anim, animDuration, defAnimFadeInTime );
	mAnimator->playDefaultAnim( time_value(0) );

	CSkinMesh* skinMesh = RGET_SKIN( name );
	mSkinUpdater = new CSkinUpdater( skinMesh->getSkeleton(), *mAnimator );
	
	CRenderableSkin* rskin = new CRenderableSkin( *skinMesh, 0,
			mSkinUpdater->getSkinMatrices(), "mSkin", "iBones",
			&mAnimator->getRootMatrix().getOrigin(), 0 );
	CEffectParams& ep = rskin->getParams();
	ep.setEffect( *RGET_FX("skin") );
	ep.addTexture( "tNormalAO", *RGET_TEX("Bicas") );
	mMesh = rskin;

	//
	// position

	SMatrix4x4& m = mAnimator->getRootMatrix();
	m.identify();
}

CComplexStuffEntity::~CComplexStuffEntity()
{
	safeDelete( mMesh );
	safeDelete( mSkinUpdater );
	safeDelete( mAnimator );
}

void CComplexStuffEntity::render()
{
	G_RENDERCTX->attach( *mMesh );
}

void CComplexStuffEntity::update( time_value timenow )
{
	mAnimator->updateLocal( timenow );
	mAnimator->updateWorld();
	mSkinUpdater->update();
}
