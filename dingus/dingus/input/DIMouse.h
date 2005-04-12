// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DI_MOUSE_H
#define __DI_MOUSE_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "InputDevice.h"


namespace dingus {



class CDIMouse : public IInputDevice {
private:
	enum { SAMPLE_BUFFER_SIZE = 8 };
	enum { BUTTON_COUNT = 4 };

public:
	CDIMouse( HWND hwnd, IDirectInput8& di8 );

	virtual CInputEvents poll();
	
protected:
	CMouseEvent::eMouseButton dim2button( int dim );

private:
	IDirectInputDevice8*		mDIMouse;
	CMouseEvent::eMouseButton	mButtonByDim[BUTTON_COUNT];
	DIMOUSESTATE				mOldDims;
};


}; // namespace


#endif
