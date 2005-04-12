// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "EffectParamsNotifier.h"

using namespace dingus;


CEffectParamsNotifier CEffectParamsNotifier::mSingleInstance;


void CEffectParamsNotifier::notify()
{
	TListenerVector::iterator it, itEnd = getListeners().end();
	for( it = getListeners().begin(); it != itEnd; ++it ) {
		CEffectParams* ep = *it;
		assert( ep );
		ep->setEffect( *ep->getEffect(), true );
	}
}
