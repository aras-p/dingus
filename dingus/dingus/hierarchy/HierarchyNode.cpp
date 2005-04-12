// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "HierarchyNode.h"

using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CHierarchyNode,64,false);
DEFINE_POOLED_ALLOC(dingus::CHierarchyContainer,64,false);


// --------------------------------------------------------------------------

void CHierarchyNode::updateHierarchy()
{
	if( !mWorldDirty )
		return;
	if( hasParent() )
		mWorldMatrix = mMatrix * mParent->getWorldMatrix();
	else
		mWorldMatrix = mMatrix;
}

// --------------------------------------------------------------------------

void CHierarchyContainer::addNode( CHierarchyNode& node )
{
	// check if in another container
	if( node.hasParent() ) {
		ASSERT_FAIL_MSG( "Already in container" );
		return;
	}

	mNodes.push_back( &node );
	node.setParent( this );
}

void CHierarchyContainer::removeNode( CHierarchyNode& node )
{
	// check if in this container
	if( node.getParent() != this ) {
		ASSERT_FAIL_MSG( "not in this container" );
		return;
	}
	mNodes.remove( &node );
	node.setParent( NULL );
}

void CHierarchyContainer::updateHierarchy()
{
	// update self
	CHierarchyNode::updateHierarchy();

	// update children
	TNodeVector::iterator it, itEnd = mNodes.end();
	for( it = mNodes.begin(); it != itEnd; ++it ) {
		CHierarchyNode& e = **it;
		assert( &e );
		e.updateHierarchy();
	}
}
