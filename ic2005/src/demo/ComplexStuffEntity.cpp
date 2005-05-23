#include "stdafx.h"
#include "ComplexStuffEntity.h"


// --------------------------------------------------------------------------


CComplexStuffEntity::CComplexStuffEntity( const char* name, const char* name2, const char* defaultAnim, float defAnimFadeInTime )
:	mLightPos(LIGHT_POS_1)
{
	mAnimator = new CCharacterAnimator();
	CAnimationBunch& anim = *RGET_ANIM(defaultAnim);
	float animDuration = gGetAnimDuration(anim,false);
	mAnimator->setDefaultAnim( anim, animDuration, defAnimFadeInTime );
	mAnimator->playDefaultAnim( CSystemTimer::getInstance().getTime() );

	CSkinMesh* skinMesh = RGET_SKIN( name );
	mSkinUpdater = new CSkinUpdater( skinMesh->getSkeleton(), *mAnimator );
	
	CSkinMesh* skinMesh2 = NULL;
	if( name2 ) {
		skinMesh2 = RGET_SKIN( name2 );
		mSkinUpdater2 = new CSkinUpdater( skinMesh2->getSkeleton(), *mAnimator );
	} else {
		mSkinUpdater2 = NULL;
	}
	
	for( int i = 0; i < RMCOUNT; ++i ) {
		mMesh[i] = NULL;
		CModelDesc* desc = RTRYGET_MDESC( RMODE_PREFIX[i] + std::string(name) );
		if( desc ) {
			CRenderableSkin* rskin = new CRenderableSkin( *skinMesh, 0,
					mSkinUpdater->getSkinMatrices(), "mSkin", "iBones",
					&mAnimator->getRootMatrix().getOrigin(), desc->getRenderPriority(0) );
			CEffectParams& ep = rskin->getParams();
			desc->fillFxParams( 0, ep );
			ep.addVector3Ref( "vLightPos", mLightPos );
			mMesh[i] = rskin;
		}

		mMesh2[i] = NULL;
		if( name2 ) {
			desc = RTRYGET_MDESC( RMODE_PREFIX[i] + std::string(name2) );
			if( desc ) {
				CRenderableSkin* rskin = new CRenderableSkin( *skinMesh2, 0,
						mSkinUpdater2->getSkinMatrices(), "mSkin", "iBones",
						&mAnimator->getRootMatrix().getOrigin(), desc->getRenderPriority(0) );
				CEffectParams& ep = rskin->getParams();
				desc->fillFxParams( 0, ep );
				ep.addVector3Ref( "vLightPos", mLightPos );
				mMesh2[i] = rskin;
			}
		}
	}

	//
	// position

	SMatrix4x4& m = mAnimator->getRootMatrix();
	m.identify();
}

CComplexStuffEntity::~CComplexStuffEntity()
{
	for( int i = 0; i < RMCOUNT; ++i ) {
		safeDelete( mMesh[i] );
		safeDelete( mMesh2[i] );
	}
	safeDelete( mSkinUpdater );
	safeDelete( mSkinUpdater2 );
	safeDelete( mAnimator );
}

void CComplexStuffEntity::render( eRenderMode renderMode )
{
	if( mMesh[renderMode] )
		G_RENDERCTX->attach( *mMesh[renderMode] );
	if( mMesh2[renderMode] )
		G_RENDERCTX->attach( *mMesh2[renderMode] );
}

void CComplexStuffEntity::update( time_value timenow )
{
	mAnimator->updateLocal( timenow );
	mAnimator->updateWorld();
	mSkinUpdater->update();
	if( mSkinUpdater2 )
		mSkinUpdater2->update();
}
