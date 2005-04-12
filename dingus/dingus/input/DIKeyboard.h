// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __DI_KEYBOARD_H
#define __DI_KEYBOARD_H

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "InputDevice.h"

namespace dingus {



class CDIKeyboard : public IInputDevice {
private:
	enum { SAMPLE_BUFFER_SIZE = 8 };
	enum { ASCII_BY_DIK_TABLE_SIZE = 256 };

public:
	CDIKeyboard( HWND hwnd, IDirectInput8& di8 );

	virtual CInputEvents poll();
	
protected:
	int dik2ascii( int dik );

private:
	IDirectInputDevice8*	mDIKeyboard;
	int						mAsciiByDik[ASCII_BY_DIK_TABLE_SIZE];
	BYTE					mOldDiks[256];
};


}; // namespace

#endif
