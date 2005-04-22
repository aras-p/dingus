#ifndef __QUADTREE_H
#define __QUADTREE_H

#include "AABox2.h"



template<typename T, int DIVAX=2>
class CQuadTreeNode : public boost::noncopyable {
public:
	typedef typename CQuadTreeNode<T,DIVAX> this_type;
	enum { DIVS_SIDE = DIVAX, DIVS_TOTAL = DIVAX*DIVAX };

public:
	static this_type* create( const SVector2& bmin, const SVector2& bmax, int depth, int* outNodeCount = NULL );

	void	init( const SVector2& bmin, const SVector2& bmax, this_type* parent, int depth, this_type*& nodes );
	
	this_type*	getBlock( const CAABox2& aabb );

	const T& getData() const { return mData; }
	T& getData() { return mData; }

private:
	bool		enclosesBox( const CAABox2& aabb ) const;
	this_type*	getBlockNoUp( const CAABox2& aabb );

private:
	SVector2	mMin;
	SVector2	mMax;
	this_type*	mParent;
	this_type*	mChildren;

	T			mData;
};



// ---- implementation ------------------------------------------------------


template<typename T, int DIVAX>
CQuadTreeNode<T,DIVAX>* CQuadTreeNode<T,DIVAX>::create( const SVector2& bmin, const SVector2& bmax, int depth, int* outNodeCount )
{
	assert( depth > 0 );

	int nodeCount = 0;
	for( int i = 0; i <= depth; ++i ) {
		nodeCount += (int)pow( DIVS_TOTAL, i );
	}

	if( outNodeCount )
		*outNodeCount = nodeCount;

	this_type* nodes = new this_type[ nodeCount ];
	assert( nodes );

	this_type* nodePtr = nodes + 1;
	nodes[0].init( bmin, bmax, NULL, depth, nodePtr );

	return nodes;
}



template<typename T, int DIVAX>
void CQuadTreeNode<T,DIVAX>::init( const SVector2& bmin, const SVector2& bmax, this_type* parent, int depth, this_type*& nodes )
{
	mMin = bmin;
	mMax = bmax;

	mParent = parent;

	if( depth <= 0 ) {
		mChildren = NULL;
		return;
	}

	mChildren = nodes;
	nodes += DIVS_TOTAL;

	SVector2 childSize = (bmax-bmin) / DIVS_SIDE;
	SVector2 childMin, childMax;

	int index = 0;
	childMin.y = bmin.y;
	for( int i = 0; i < DIVS_SIDE; ++i ) {
		childMax.y = (i==DIVS_SIDE-1) ? (bmax.y) : (childMin.y + childSize.y);
		
		childMin.x = bmin.x;
		for( int j = 0; j < DIVS_SIDE; ++j ) {
			childMax.x = (j==DIVS_SIDE-1) ? (bmax.x) : (childMin.x + childSize.x);

			mChildren[index].init( childMin, childMax, this, depth-1, nodes );

			childMin.x += childSize.x;
			++index;
		}

		childMin.y += childSize.y;
	}
}


template<typename T, int DIVAX>
bool  CQuadTreeNode<T,DIVAX>::enclosesBox( const CAABox2& aabb ) const
{
	return
		aabb.getMin().x >= mMin.x &&
		aabb.getMax().x <= mMax.x &&
		aabb.getMin().y >= mMin.y &&
		aabb.getMax().y <= mMax.y;
}


template<typename T, int DIVAX>
CQuadTreeNode<T,DIVAX>*	CQuadTreeNode<T,DIVAX>::getBlockNoUp( const CAABox2& aabb )
{
	if( mChildren ) {
		for( int i = 0; i < DIVS_TOTAL; ++i ) {
			if( mChildren[i].enclosesBox( aabb ) )
				return mChildren[i].getBlockNoUp( aabb ); // child will have good node
		}
	}
	return this; // we are the best node
}


template<typename T, int DIVAX>
CQuadTreeNode<T,DIVAX>*	CQuadTreeNode<T,DIVAX>::getBlock( const CAABox2& aabb )
{
	if( enclosesBox( aabb ) ) {
		return getBlockNoUp( aabb ); // this or children will have a good node
	} else if( mParent ) {
		return mParent->getBlock( aabb ); // parent has a good block
	} else
		return this; // we are the root, no other choice
}


#endif
