// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __HIER_CONTEXT_H
#define __HIER_CONTEXT_H

#include "Hierarchable.h"

namespace dingus {

class CHierarchyContext {
public:
	typedef	fastvector<IHierarchable*> THierVector;

	void addHierarchable( IHierarchable& h ) { mHierarchables.push_back(&h); }
	void removeHierarchable( IHierarchable& h ) { mHierarchables.remove(&h); }

	void perform() {
		THierVector::iterator it, itEnd = mHierarchables.end();
		for( it = mHierarchables.begin(); it != itEnd; ++it ) {
			assert( *it );
			(*it)->updateHierarchy();
		}
	}

private:
	THierVector mHierarchables;
};

}; // namespace

#endif
