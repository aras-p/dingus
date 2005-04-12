// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VisibilityDB.h"
#include "VisibilityLeaf.h"

using namespace dingus;


// --------------------------------------------------------------------------
//  CAbstractVisibilityDB


void CAbstractVisibilityDB::process()
{
	if( mEnabled )
		processContent();
}


// --------------------------------------------------------------------------
//  CDummyVisibilityDB


void CDummyVisibilityDB::processContent()
{
	TSlotVector::iterator it, itEnd = getLeafs().end();
	for( it = getLeafs().begin(); it != itEnd; ++it ) {
		assert( *it );
		(*it)->setVisible( *this );
	}
}


// --------------------------------------------------------------------------
//  CFrustumVisibilityDB


void CFrustumVisibilityDB::processContent()
{
	assert( mViewProjection );
	TSlotVector::iterator it, itEnd = getLeafs().end();
	for( it = getLeafs().begin(); it != itEnd; ++it ) {
		CVisibilityLeaf* leaf = *it;
		assert( leaf );
		if( leaf->testVisible( *mViewProjection ) )
			leaf->setVisible( *this );
	}
}
