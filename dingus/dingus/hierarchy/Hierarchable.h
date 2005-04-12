// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __HIERARCHABLE_H
#define __HIERARCHABLE_H

namespace dingus {

// --------------------------------------------------------------------------

class IHierarchable {
public:
	virtual ~IHierarchable() = 0 { };

	virtual void updateHierarchy() = 0;
};

}; // namespace


#endif
