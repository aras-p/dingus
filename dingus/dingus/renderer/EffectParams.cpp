// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "EffectParams.h"
#include "EffectParamsNotifier.h"

using namespace dingus;


CEffectParams::CEffectParams()
:	mEffect(0)
{
	CEffectParamsNotifier::getInstance().addListener( *this );
}

CEffectParams::~CEffectParams()
{
	CEffectParamsNotifier::getInstance().removeListener( *this );
}

CEffectParams::CEffectParams( const CEffectParams& rhs )
:	mEffect(rhs.mEffect),
	mParams1(rhs.mParams1),
	mParams3(rhs.mParams3),
	mParams4(rhs.mParams4),
	mParams16(rhs.mParams16),
	mParamsPtr(rhs.mParamsPtr),
	mParamsProxy(rhs.mParamsProxy)
{
	CEffectParamsNotifier::getInstance().addListener( *this );
}

const CEffectParams& CEffectParams::operator=( const CEffectParams& rhs )
{
	mEffect = rhs.mEffect;
	mParams1 = rhs.mParams1;
	mParams3 = rhs.mParams3;
	mParams4 = rhs.mParams4;
	mParams16 = rhs.mParams16;
	mParamsPtr = rhs.mParamsPtr;
	mParamsProxy = rhs.mParamsProxy;
	return *this;
}


void CEffectParams::initHandle( SParamHandle& par ) const
{
	if( !mEffect )
		return;
	par.handle = mEffect->getObject()->GetParameterByName( NULL, par.name.c_str() );
}


void CEffectParams::addInt( TParamName name, int i )
{
	mParams1.push_back( SParam1() );
	SParam1& par = mParams1.back();
	par.name = name;
	initHandle( par );
	par.valI = i;
}

void CEffectParams::addVector3( TParamName name, const SVector3& v )
{
	mParams3.push_back( SParam3() );
	SParam3& par = mParams3.back();
	par.name = name;
	initHandle( par );
	par.valV = v;
}

void CEffectParams::addVector4( TParamName name, const SVector4& v )
{
	mParams4.push_back( SParam4() );
	SParam4& par = mParams4.back();
	par.name = name;
	initHandle( par );
	par.valV = v;
}

void CEffectParams::addMatrix4x4( TParamName name, const SMatrix4x4& m )
{
	mParams16.push_back( SParam16() );
	SParam16& par = mParams16.back();
	par.name = name;
	initHandle( par );
	par.valM = m;
}

void CEffectParams::addPtr( TParamName name, int size, const void* ptr )
{
	mParamsPtr.push_back( SParamPtr() );
	SParamPtr& par = mParamsPtr.back();
	par.name = name;
	initHandle( par );
	par.size = size;
	par.ptr = ptr;
}

void CEffectParams::addProxy( TParamName name, const CBaseProxyClass& proxy )
{
	mParamsProxy.push_back( SParam1() );
	SParam1& par = mParamsProxy.back();
	par.name = name;
	initHandle( par );
	par.proxy = &proxy;
}

void CEffectParams::removeParam1( TParamName name )
{
	TParam1Vector::iterator last = mParams1.end();
	for( TParam1Vector::iterator first = mParams1.begin(); first != last; ) {
		if( !strcmp( first->name.c_str(), name ) )
			first = mParams1.erase(first), --last;
		else
			++first;
	}
}
void CEffectParams::removeParam3( TParamName name )
{
	TParam3Vector::iterator last = mParams3.end();
	for( TParam3Vector::iterator first = mParams3.begin(); first != last; ) {
		if( !strcmp( first->name.c_str(), name ) )
			first = mParams3.erase(first), --last;
		else
			++first;
	}
}
void CEffectParams::removeParam4( TParamName name )
{
	TParam4Vector::iterator last = mParams4.end();
	for( TParam4Vector::iterator first = mParams4.begin(); first != last; ) {
		if( !strcmp( first->name.c_str(), name ) )
			first = mParams4.erase(first), --last;
		else
			++first;
	}
}
void CEffectParams::removeParam16( TParamName name )
{
	TParam16Vector::iterator last = mParams16.end();
	for( TParam16Vector::iterator first = mParams16.begin(); first != last; ) {
		if( !strcmp( first->name.c_str(), name ) )
			first = mParams16.erase(first), --last;
		else
			++first;
	}
}
void CEffectParams::removeParamPtr( TParamName name )
{
	TParamPtrVector::iterator last = mParamsPtr.end();
	for( TParamPtrVector::iterator first = mParamsPtr.begin(); first != last; ) {
		if( !strcmp( first->name.c_str(), name ) )
			first = mParamsPtr.erase(first), --last;
		else
			++first;
	}
}
void CEffectParams::removeParamProxy( TParamName name )
{
	TParamProxyVector::iterator last = mParamsProxy.end();
	for( TParamProxyVector::iterator first = mParamsProxy.begin(); first != last; ) {
		SParam1& p = *first;
		if( !strcmp( p.name.c_str(), name ) ) {
			// unset from effect (otherwise we can leak)
			if( p.handle ) {
				assert( mEffect );
				ID3DXEffect* fx = mEffect->getObject();
				assert( fx );
				const void* ptr = NULL;
				fx->SetValue( p.handle, &ptr, sizeof(ptr) );
			}
			first = mParamsProxy.erase(first), --last;
		} else
			++first;
	}
}

void CEffectParams::clear()
{
	mParams1.clear();
	mParams3.clear();
	mParams4.clear();
	mParams16.clear();
	mParamsPtr.clear();
	mParamsProxy.clear();
}

void CEffectParams::setEffect( CD3DXEffect& fx, bool force )
{
	// early out if effect is the same
	if( !force && mEffect && (fx.getObject() == mEffect->getObject()) )
		return;

	mEffect = &fx;
	if( !mEffect )
		return;
	assert( mEffect && mEffect->getObject() );

	// recalculate parameter handles
	TParam1Vector::iterator it1, it1end = mParams1.end();
	for( it1 = mParams1.begin(); it1 != it1end; ++it1 ) {
		initHandle( *it1 );
	}
	TParam3Vector::iterator it3, it3end = mParams3.end();
	for( it3 = mParams3.begin(); it3 != it3end; ++it3 ) {
		initHandle( *it3 );
	}
	TParam4Vector::iterator it4, it4end = mParams4.end();
	for( it4 = mParams4.begin(); it4 != it4end; ++it4 ) {
		initHandle( *it4 );
	}
	TParam16Vector::iterator it16, it16end = mParams16.end();
	for( it16 = mParams16.begin(); it16 != it16end; ++it16 ) {
		initHandle( *it16 );
	}
	TParamPtrVector::iterator itPtr, itPtrend = mParamsPtr.end();
	for( itPtr = mParamsPtr.begin(); itPtr != itPtrend; ++itPtr ) {
		initHandle( *itPtr );
	}
	TParamProxyVector::iterator itPrx, itPrxend = mParamsProxy.end();
	for( itPrx = mParamsProxy.begin(); itPrx != itPrxend; ++itPrx ) {
		initHandle( *itPrx );
	}
}

void CEffectParams::applyToEffect()
{
	assert( mEffect );
	ID3DXEffect* fx = mEffect->getObject();
	assert( fx );

	TParam1Vector::iterator it1, it1end = mParams1.end();
	for( it1 = mParams1.begin(); it1 != it1end; ++it1 ) {
		SParam1& p = *it1;
		if( !p.handle ) continue; // unused
		fx->SetValue( p.handle, &p.valI, sizeof(p.valI) );
	}
	TParam3Vector::iterator it3, it3end = mParams3.end();
	for( it3 = mParams3.begin(); it3 != it3end; ++it3 ) {
		SParam3& p = *it3;
		if( !p.handle ) continue; // unused
		fx->SetValue( p.handle, &p.valV, sizeof(p.valV) );
	}
	TParam4Vector::iterator it4, it4end = mParams4.end();
	for( it4 = mParams4.begin(); it4 != it4end; ++it4 ) {
		SParam4& p = *it4;
		if( !p.handle ) continue; // unused
		fx->SetValue( p.handle, &p.valV, sizeof(p.valV) );
	}
	TParam16Vector::iterator it16, it16end = mParams16.end();
	for( it16 = mParams16.begin(); it16 != it16end; ++it16 ) {
		SParam16& p = *it16;
		if( !p.handle ) continue; // unused
		fx->SetValue( p.handle, &p.valM, sizeof(p.valM) );
	}
	TParamPtrVector::iterator itPtr, itPtrend = mParamsPtr.end();
	for( itPtr = mParamsPtr.begin(); itPtr != itPtrend; ++itPtr ) {
		SParamPtr& p = *itPtr;
		if( !p.handle ) continue; // unused
		fx->SetValue( p.handle, p.ptr, p.size );
	}
	TParamProxyVector::iterator itProx, itProxend = mParamsProxy.end();
	for( itProx = mParamsProxy.begin(); itProx != itProxend; ++itProx ) {
		SParam1& p = *itProx;
		if( !p.handle ) continue; // unused
		const void* ptr = p.proxy->getPtr();
		fx->SetValue( p.handle, &ptr, sizeof(ptr) );
	}
}
