// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "InputContext.h"

#include "InputDevice.h"

using namespace dingus;


void CInputContext::perform()
{
	//
	// poll devices, notify listeners

	TDeviceContainer::iterator dit, ditEnd = mDevices.end();
	for( dit = mDevices.begin(); dit != ditEnd; ++dit ) {
		IInputDevice* device = *dit;
		assert( device );

		CInputEvents polled( device->poll() );
		CInputEvents::TEventsVector::const_iterator it, itEnd = polled.getEvents().end();
		for( it = polled.getEvents().begin(); it!=itEnd; ++it ) {
			const CInputEvent& event = **it;
			assert( &event );
			TListenerVector::iterator lit, litEnd = getListeners().end();
			for( lit = getListeners().begin(); lit != litEnd; ++lit )
				(*lit)->onInputEvent( event );
		}
	}

	//
	// end-stage listeners notify

	TListenerVector::iterator lit, litEnd = getListeners().end();
	for( lit = getListeners().begin(); lit != litEnd; ++lit )
		(*lit)->onInputStage();
}
