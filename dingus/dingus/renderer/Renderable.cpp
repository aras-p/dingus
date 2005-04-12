// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Renderable.h"

using namespace dingus;

void CRenderable::beforeRender( CRenderContext& ctx )
{
	TListenerVector::iterator it, itEnd = getListeners().end();
	for( it = getListeners().begin(); it != itEnd; ++it ) {
		assert( *it );
		(*it)->beforeRender( *this, ctx );
	}
}

void CRenderable::afterRender( CRenderContext& ctx )
{
	TListenerVector::iterator it, itEnd = getListeners().end();
	for( it = getListeners().begin(); it != itEnd; ++it ) {
		assert( *it );
		(*it)->afterRender( *this, ctx );
	}
}

