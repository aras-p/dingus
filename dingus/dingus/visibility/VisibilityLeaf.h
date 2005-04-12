// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VISIBILITY_LEAF_H
#define __VISIBILITY_LEAF_H

#include "../math/Matrix4x4.h"
#include "../math/AABox.h"
#include "../utils/Notifier.h"
#include "VisibilityDB.h"
#include "../utils/MemoryPool.h"

namespace dingus {


class CVisibilityLeaf;


// --------------------------------------------------------------------------

class IVisibilityListener {
public:
	virtual ~IVisibilityListener() = 0 {};
	virtual void onVisible( CVisibilityLeaf const& visLeaf ) = 0;
};


// --------------------------------------------------------------------------

class CVisibilityLeaf : public CNotifier<IVisibilityListener> {
public:
	CVisibilityLeaf() : mEnabled(true) { };
	virtual ~CVisibilityLeaf() { };

	virtual bool testVisible( const SMatrix4x4& viewProj ) const { return true; }

	void setVisible( CAbstractVisibilityDB const& db ) {
		if( mEnabled )
			notifyListeners();
	}

	void setEnabled( bool e ) { mEnabled = e; }
	bool isEnabled() const { return mEnabled; }
	
protected:
	void notifyListeners() {
		TListenerVector::iterator it, itEnd = getListeners().end();
		for( it = getListeners().begin(); it != itEnd; ++it ) {
			IVisibilityListener* l = *it;
			assert( l );
			l->onVisible( *this );
		}
	}
private:
	DECLARE_POOLED_ALLOC(dingus::CVisibilityLeaf);
private:
	bool	mEnabled;
};


// --------------------------------------------------------------------------

class CAABBVisibilityLeaf : public CVisibilityLeaf {
public:
	CAABBVisibilityLeaf( const SMatrix4x4& worldMatrix ) : mWorldMatrix(&worldMatrix) { }

	virtual bool testVisible( const SMatrix4x4& viewProj ) const;

	const CAABox& getAABB() const { return mAABB; }
	CAABox& getAABB() { return mAABB; }

	const SMatrix4x4& getWorldMatrix() const { return *mWorldMatrix; }
	void setWorldMatrix( const SMatrix4x4& worldMatrix ) { mWorldMatrix = &worldMatrix; }

private:
	DECLARE_POOLED_ALLOC(dingus::CAABBVisibilityLeaf);
private:
	CAABox				mAABB;
	const SMatrix4x4*	mWorldMatrix;
};


}; // namespace


#endif
