// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "VisibilityLeaf.h"

using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CVisibilityLeaf,256,false);
DEFINE_POOLED_ALLOC(dingus::CAABBVisibilityLeaf,256,false);


// --------------------------------------------------------------------------

bool CAABBVisibilityLeaf::testVisible( const SMatrix4x4& viewProj ) const
{
	assert( mWorldMatrix );
	bool outside = mAABB.frustumCull( *mWorldMatrix, viewProj );
	return !outside;
}
