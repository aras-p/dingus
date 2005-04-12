// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VISIBILITY_DB_H
#define __VISIBILITY_DB_H

#include "../math/Matrix4x4.h"

namespace dingus {


class CVisibilityLeaf;


/**
 *  Abstract visibility database.
 *
 *  Visibility slots are laid out flat - no spatial hierarchies are used for now.
 */
class CAbstractVisibilityDB : public CRefCounted {
public:
	typedef DingusSmartPtr<CAbstractVisibilityDB> TSharedPtr;
public:
	CAbstractVisibilityDB() : mEnabled(true) { };
	virtual ~CAbstractVisibilityDB() = 0 { };

	void process();

	void addLeaf( CVisibilityLeaf& slot ) { mLeafs.push_back(&slot); }
	void removeLeaf( CVisibilityLeaf& slot ) { mLeafs.remove(&slot); }

	int getLeafCount() const { return mLeafs.size(); }

	void setEnabled( bool e ) { mEnabled = e; }
	bool isEnabled() const { return mEnabled; }

protected:
	typedef fastvector<CVisibilityLeaf*>	TSlotVector;

protected:
	virtual void processContent() = 0;

	TSlotVector const& getLeafs() const { return mLeafs; }
	TSlotVector& getLeafs() { return mLeafs; }
	
private:
	TSlotVector		mLeafs;
	bool			mEnabled;
};


class CDummyVisibilityDB : public CAbstractVisibilityDB {
public:
	CDummyVisibilityDB() : CAbstractVisibilityDB() { }

protected:
	// CAbstractVisibilityDB
	virtual void processContent();
};


class CFrustumVisibilityDB : public CAbstractVisibilityDB {
public:
	CFrustumVisibilityDB( const SMatrix4x4& viewProj ) : CAbstractVisibilityDB(), mViewProjection(&viewProj) { };

	void setViewProjMatrix( const SMatrix4x4& viewProj ) { mViewProjection = &viewProj; }

protected:
	// CAbstractVisibilityDB
	virtual void processContent();
private:
	const SMatrix4x4*	mViewProjection;
};


}; // namespace

#endif