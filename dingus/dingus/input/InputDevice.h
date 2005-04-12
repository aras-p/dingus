// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __INPUTDEVICE_H
#define __INPUTDEVICE_H

#include "InputEvent.h"

namespace dingus {



class IInputDevice {
public:
	virtual ~IInputDevice() = 0 {};

	virtual CInputEvents poll() = 0;
};


}; // namespace

#endif
