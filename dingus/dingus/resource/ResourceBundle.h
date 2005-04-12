// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RESOURCE_BUNDLE_H
#define __RESOURCE_BUNDLE_H

#include "ResourceId.h"
#include "../console/Console.h"

namespace dingus {

class IResourceBundle {
public:
	static CConsoleChannel& CONSOLE;
public:
	virtual ~IResourceBundle() = 0 {};
};


}; // namespace

#endif
