// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __INPUT_CONTEXT_H
#define __INPUT_CONTEXT_H

#include "../utils/Notifier.h"
#include "InputListener.h"
#include "InputEvent.h"

namespace dingus {


class IInputDevice;



class CInputContext : public CNotifier<IInputListener> {
public:
	typedef fastvector<IInputDevice*> TDeviceContainer;
	
public:
	void	addDevice( IInputDevice& device ) { mDevices.push_back(&device); }
	void	removeDevice( IInputDevice& device ) { mDevices.remove(&device); }
	const TDeviceContainer& getDevices() const { return mDevices; }
	TDeviceContainer& getDevices() { return mDevices; }

	void	perform();
	
private:
	TDeviceContainer	mDevices;
};


}; // namespace

#endif
