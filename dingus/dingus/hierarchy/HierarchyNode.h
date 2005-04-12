// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __HIER_NODE_H
#define __HIER_NODE_H

#include "Hierarchable.h"
#include "../math/Matrix4x4.h"
#include "../utils/MemoryPool.h"

namespace dingus {

class CHierarchyContainer;

// --------------------------------------------------------------------------

class CHierarchyNode : public IHierarchable {
public:
	CHierarchyNode() : mWorldDirty(true), mParent(0) { mMatrix.identify(); };
	virtual ~CHierarchyNode() = 0 { };

	SMatrix4x4 const& getMatrix() const { return mMatrix; }
	SMatrix4x4& getMatrix() { mWorldDirty = true; return mMatrix; }
	SMatrix4x4 const& getWorldMatrix() const { return mWorldMatrix; }

	void setParent( CHierarchyContainer* p ) { mWorldDirty = true; mParent = p; }
	const CHierarchyContainer* getParent() const { return mParent; }
	CHierarchyContainer* getParent() { return mParent; }
	bool hasParent() const { return (mParent != 0); }

	// IHierarchable
	virtual void updateHierarchy();

private:
	DECLARE_POOLED_ALLOC(dingus::CHierarchyNode);
private:
	SMatrix4x4		mMatrix;
	SMatrix4x4		mWorldMatrix;
	CHierarchyContainer* mParent;
	bool			mWorldDirty;
};


// --------------------------------------------------------------------------

class CHierarchyContainer : public CHierarchyNode {
public:
	typedef fastvector<CHierarchyNode*> TNodeVector;
public:
	void	addNode( CHierarchyNode& node );
	void	removeNode( CHierarchyNode& node );

	const TNodeVector& getNodes() const { return mNodes; }
	int getNodeCount() const { return mNodes.size(); }
	const CHierarchyNode& getNode( int idx ) const { return *mNodes[idx]; }
	CHierarchyNode& getNode( int idx ) { return *mNodes[idx]; }

	// IHierarchable
	virtual void updateHierarchy();

private:
	DECLARE_POOLED_ALLOC(dingus::CHierarchyNode);
private:
	TNodeVector		mNodes;
};


}; // namespace


#endif
