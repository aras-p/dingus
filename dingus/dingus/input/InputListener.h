// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __INPUT_LISTENER_H
#define __INPUT_LISTENER_H

namespace dingus {

class CInputEvent;


class IInputListener {
public:
	virtual ~IInputListener() =0 {};

	/**
	 *  Called by context on input event.
	 */
	virtual void onInputEvent( const CInputEvent& event ) = 0;

	/**
	 *  Called by context at the end of input stage.
	 */
	virtual void onInputStage() = 0;
};


}; // namespace


#endif
