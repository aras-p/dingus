// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "ProxyEffect.h"
#include "D3DDevice.h"
#include "../console/Console.h"
#include "../utils/Errors.h"

using namespace dingus;


CD3DXEffect::CD3DXEffect( ID3DXEffect* object )
:	CBaseProxyClass(object)
{
	if( object )
		init();
}

void CD3DXEffect::setObject( ID3DXEffect* object )
{
	setPtr(object);
	if( object )
		init();
}

void CD3DXEffect::init()
{
	CD3DDevice& dx = CD3DDevice::getInstance();
	mSoftwareVertexProcessed = false;
	bool devNonSW = dx.getCaps().getVertexProcessing() != CD3DDeviceCaps::VP_SW;

	if( devNonSW )
		dx.getDevice().SetSoftwareVertexProcessing( FALSE );
	bool okhwvp = tryInit();
	if( !okhwvp ) {
		mSoftwareVertexProcessed = true;
		if( devNonSW )
			dx.getDevice().SetSoftwareVertexProcessing( TRUE );
		bool okswvp = tryInit();
		if( !okswvp ) {
			// no valid technique found, throw exception
			std::string msg = "no valid techniques found in effect";
			CConsole::CON_ERROR.write( msg );
			THROW_ERROR( msg );
		}
		if( devNonSW )
			dx.getDevice().SetSoftwareVertexProcessing( FALSE );
	}
}

bool CD3DXEffect::isValidTechnique( D3DXHANDLE tech )
{
	ID3DXEffect* fx = getObject();
	assert( fx );

	// validate the technique
	if( FAILED( fx->ValidateTechnique( tech ) ) )
		return false;

	CD3DDevice& dx = CD3DDevice::getInstance();
	D3DXHANDLE annot;

	// needs shadow map support?
	annot = fx->GetAnnotationByName( tech, "shadowMap" );
	if( annot != NULL ) {
		BOOL val = FALSE;
		fx->GetBool( annot, &val );
		if( val && !dx.getCaps().hasShadowMaps() )
			return false;
	}

	// needs floating point RT support?
	annot = fx->GetAnnotationByName( tech, "floatTexture" );
	if( annot != NULL ) {
		BOOL val = FALSE;
		fx->GetBool( annot, &val );
		if( val && !dx.getCaps().hasFloatTextures() )
			return false;
	}

	// all ok!
	return true;
}

bool CD3DXEffect::tryInit()
{
	HRESULT hres;

	ID3DXEffect* fx = getObject();
	assert( fx );

	//
	// find a valid technique

	D3DXHANDLE tech = fx->GetTechnique( 0 );
	while( tech && !isValidTechnique( tech ) ) {
		hres = fx->FindNextValidTechnique( tech, &tech );
	}
	if( tech != NULL )
		fx->SetTechnique( tech );
	else
		return false;
	assert( tech != NULL );

	//
	// back to front?

	D3DXHANDLE annot = fx->GetAnnotationByName( tech, "backToFront" );
	BOOL backToFront = FALSE;
	if( annot != NULL )
		fx->GetBool( annot, &backToFront );
	mBackToFrontSorted = backToFront ? true : false;

	// sort value
	mSortValue = 0;
	mSortValue += mBackToFrontSorted ? 0x10 : 0x00;
	mSortValue += mSoftwareVertexProcessed ? 0 : 1;

	//
	// has restoring pass?

	D3DXTECHNIQUE_DESC techdsc;
	fx->GetTechniqueDesc( tech, &techdsc );
	D3DXPASS_DESC passdsc;
	fx->GetPassDesc( fx->GetPass(tech,techdsc.Passes-1), &passdsc );
	mHasRestoringPass = !stricmp( passdsc.Name, DINGUS_FX_RESTORE_PASS );
	assert( mHasRestoringPass );
	mPassCount = mHasRestoringPass ? techdsc.Passes-1 : techdsc.Passes;

	return true;
}

// --------------------------------------------------------------------------

#define FX_BEGIN_PARAMS (D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE)
#define FX_USE_RESTORING_PASS

// if using this, uncomment DISABLE_FILTERING in D3DDevice.cpp and EffectStateManager.cpp!
//#define FX_BEGIN_PARAMS (0)


int CD3DXEffect::beginFx()
{
	UINT p;
	DWORD flags = mHasRestoringPass ? (FX_BEGIN_PARAMS) : 0;
	HRESULT hr = getObject()->Begin( &p, flags );
	assert( SUCCEEDED(hr) );
	assert( mHasRestoringPass && p==mPassCount+1 || !mHasRestoringPass && p==mPassCount );
	return mPassCount;
}

void CD3DXEffect::endFx()
{
#ifdef FX_USE_RESTORING_PASS
	if( mHasRestoringPass ) {
		beginPass( mPassCount );
		endPass();
	}
#endif
	HRESULT hr = getObject()->End();
	assert( SUCCEEDED(hr) );
}

void CD3DXEffect::beginPass( int p )
{
#ifdef USE_D3DX_SUMMER_2004
	HRESULT hr = getObject()->BeginPass( p );
#else
	HRESULT hr = getObject()->Pass( p );
#endif
	assert( SUCCEEDED(hr) );
}

void CD3DXEffect::endPass()
{
#ifdef USE_D3DX_SUMMER_2004
	HRESULT hr = getObject()->EndPass();
	assert( SUCCEEDED(hr) );
#endif
}

void CD3DXEffect::commitParams()
{
#ifdef USE_D3DX_SUMMER_2004
	HRESULT hr = getObject()->CommitChanges();
	assert( SUCCEEDED(hr) );
#endif
}
