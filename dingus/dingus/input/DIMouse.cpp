// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "DIMouse.h"

using namespace dingus;


struct SDIMToButtonMap
{
	int							dim;
	CMouseEvent::eMouseButton	button;
};

SDIMToButtonMap gDim2Button [] = 
{	{	0,	CMouseEvent::LEFT_BTN },
	{	1,	CMouseEvent::RIGHT_BTN },
	{	2,	CMouseEvent::MIDDLE_BTN }
};
int gDim2ButtonCount = sizeof(gDim2Button) / sizeof(gDim2Button[0]);


CDIMouse::CDIMouse( HWND hwnd, IDirectInput8& di8 )
:	mDIMouse( NULL )
{
	HRESULT hRes;
	DWORD   dwCoopFlags;

	dwCoopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;
//	dwCoopFlags = DISCL_EXCLUSIVE | DISCL_FOREGROUND;
	
	// Obtain an interface to the system mouse device.
	hRes = di8.CreateDevice(
		GUID_SysMouse,
		&mDIMouse,
		NULL );
	assert( SUCCEEDED( hRes ) );
	assert( mDIMouse );
	
    
    // Set the data format to "mouse format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing a
    // DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
	hRes = mDIMouse->SetDataFormat( &c_dfDIMouse );
	assert( SUCCEEDED( hRes ) );
	
	// Set the mouse axis mode to relative
	// See "Interpreting Mouse Axis Data" topic
	DIPROPDWORD diprop;
	diprop.diph.dwSize = sizeof( diprop );
	diprop.diph.dwHeaderSize = sizeof( diprop.diph );
	diprop.diph.dwObj = 0;
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.dwData = DIPROPAXISMODE_REL;
    hRes = mDIMouse->SetProperty( DIPROP_AXISMODE, &diprop.diph );
	assert( SUCCEEDED( hRes ) );

	
	// Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
	hRes = mDIMouse->SetCooperativeLevel( hwnd, dwCoopFlags );
	assert( SUCCEEDED( hRes ) );

	for( int q = 0; q < gDim2ButtonCount; q++ )
	{
		int index = ::gDim2Button[q].dim;
		assert( index >= 0 && index < BUTTON_COUNT );
		
		mButtonByDim[ index ] = ::gDim2Button[q].button;
	}
	
/*	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
	//
	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
	// If you want to read buffered data, you need to set a nonzero
	// buffer size.
	//
	// Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
	//
	// The buffer size is a DWORD property associated with the device.
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

	if( FAILED( hr = mDirectInput8Keyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
		throw EError( "keyboard device buffer size setting failed" );
*/
	// Acquire the newly created device
	mDIMouse->Acquire();
}


CMouseEvent::eMouseButton CDIMouse::dim2button( int dim )
{
	return mButtonByDim[ dim ];
}

CInputEvents CDIMouse::poll()
{
	HRESULT hr;
    DIMOUSESTATE dims;      // DirectInput mouse state structure

	CInputEvents events;
	
	// Get the input's device state, and put the state in dims
	ZeroMemory( &dims, sizeof(dims) );
	if( FAILED( hr = mDIMouse->GetDeviceState( sizeof(dims), &dims ) ) )
	{
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        hr = mDIMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = mDIMouse->Acquire();

        //if( hr == DIERR_OTHERAPPHASPRIO || 
        //    hr == DIERR_NOTACQUIRED ) 

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
		return events;
	}

	long x = dims.lX;
	long y = dims.lY;

	long ox = mOldDims.lX;
	long oy = mOldDims.lY;
	
	for( int i = 0; i < BUTTON_COUNT; i++ ) {
		CMouseEvent::eMouseButton btn = dim2button(i);
		CMouseEvent::eMode mode;

		if( dims.rgbButtons[i] != mOldDims.rgbButtons[i] ) {
			mode = (dims.rgbButtons[i]&0x80) ? (CMouseEvent::MOUSE_PRESSED) : (CMouseEvent::MOUSE_RELEASED);
		} else {
			if( dims.rgbButtons[i] & 0x80 ) {
				mode = (ox!=x || oy!=y) ? (CMouseEvent::MOUSE_DRAGGED) : (CMouseEvent::MOUSE_DOWN);
			} else {
				mode = (ox!=x || oy!=y) ? (CMouseEvent::MOUSE_MOVE) : (CMouseEvent::MOUSE_NO_ACTION);
			}
			events.addEvent( new CMouseEvent(mode,btn,x,y,1) );
		}

	}
	mOldDims = dims;

	return events;
}