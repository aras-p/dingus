// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "DIKeyboard.h"

using namespace dingus;



int gDik2Ascii [][2] = 
{	{	DIK_ESCAPE,	0x27 },
	{	DIK_SPACE,	' ' },
	{	DIK_0,	'0' },
	{	DIK_1,	'1' },
	{	DIK_2,	'2' },
	{	DIK_3,	'3' },
	{	DIK_4,	'4' },
	{	DIK_5,	'5' },
	{	DIK_6,	'6' },
	{	DIK_7,	'7' },
	{	DIK_8,	'8' },
	{	DIK_9,	'9' },
	{	DIK_Q,	'Q'	},
	{	DIK_W,	'W'	},
	{	DIK_E,	'E'	},
	{	DIK_R,	'R'	},
	{	DIK_T,	'T'	},
	{	DIK_Y,	'Y'	},
	{	DIK_U,	'U'	},
	{	DIK_I,	'I'	},
	{	DIK_O,	'O'	},
	{	DIK_P,	'P'	},
	{	DIK_A,	'A'	},
	{	DIK_S,	'S'	},
	{	DIK_D,	'D'	},
	{	DIK_F,	'F'	},
	{	DIK_G,	'G'	},
	{	DIK_H,	'H'	},
	{	DIK_J,	'J'	},
	{	DIK_K,	'K'	},
	{	DIK_L,	'L'	},
	{	DIK_Z,	'Z'	},
	{	DIK_X,	'X'	},
	{	DIK_C,	'C'	},
	{	DIK_V,	'V'	},
	{	DIK_B,	'B'	},
	{	DIK_N,	'N'	},
	{	DIK_M,	'M'	}	
};
int gDikCount = sizeof(gDik2Ascii) / sizeof(gDik2Ascii[0]);

CDIKeyboard::CDIKeyboard( HWND hwnd, IDirectInput8& di8 )
:	mDIKeyboard( NULL )
{
	HRESULT hRes;
	DWORD   dwCoopFlags;

	dwCoopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;

	// Obtain an interface to the system keyboard device.
	hRes = di8.CreateDevice(
		GUID_SysKeyboard,
		&mDIKeyboard,
		NULL );
	assert( SUCCEEDED( hRes ) );
	assert( mDIKeyboard );
	
	// hr the data format to "keyboard format" - a predefined data format 
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing an array
	// of 256 bytes to IDirectInputDevice::GetDeviceState.
	hRes = mDIKeyboard->SetDataFormat( &c_dfDIKeyboard );
	assert( SUCCEEDED( hRes ) );
	
	// Set the cooperativity level to let DirectInput know how
	// this device should interact with the system and with other
	// DirectInput applications.
	hRes = mDIKeyboard->SetCooperativeLevel( hwnd, dwCoopFlags );
	assert( SUCCEEDED( hRes ) );

	int q;
	for( q = 0; q < ASCII_BY_DIK_TABLE_SIZE; q++ )
	{
		mAsciiByDik[ q ] = 0;
	}

	for( q = 0; q < gDikCount; q++ )
	{
		int index = ::gDik2Ascii[q][0];
		assert( index > 0 && index < ASCII_BY_DIK_TABLE_SIZE );

		mAsciiByDik[ index ] = ::gDik2Ascii[q][1];
	}

	memset( mOldDiks, 0x00, sizeof( mOldDiks ) );

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

	if( FAILED( hRes = mDIKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
	assert( SUCCEEDED( hRes ) );
*/
	// Acquire the newly created device
	mDIKeyboard->Acquire();
}



int CDIKeyboard::dik2ascii( int dik )
{
	if( dik >= 0 && dik < ASCII_BY_DIK_TABLE_SIZE )
		return mAsciiByDik[ dik ];
	else
		return 0;
}

CInputEvents CDIKeyboard::poll()
{
	HRESULT hr;
	BYTE	diks[256];	 // DirectInput keyboard state buffer
	int 	i;

	CInputEvents events;
	
	// Get the input's device state, and put the state in diks
	ZeroMemory( &diks, sizeof(diks) );
	if( FAILED( hr = mDIKeyboard->GetDeviceState( sizeof(diks), &diks ) ) )
	{
		// DirectInput may be telling us that the input stream has been
		// interrupted.  We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
		
		// If input is lost then acquire and keep trying 
		mDIKeyboard->Acquire();
		while( hr == DIERR_INPUTLOST ) 
			hr = mDIKeyboard->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return events;
	}

	for( i = 0; i < 256; i++ ) {
		if( diks[i] == mOldDiks[i] && !(diks[i]&0x80) )
			continue;

		int keyCode = dik2ascii(i);
		CKeyEvent::eMode mode;
		if( diks[i] == mOldDiks[i] )
			mode = CKeyEvent::KEY_DOWN;
		else
			mode = (diks[i]&0x80) ? (CKeyEvent::KEY_PRESSED) : (CKeyEvent::KEY_RELEASED);
		events.addEvent( new CKeyEvent(mode,keyCode,i) );

		mOldDiks[i] = diks[i];
	}
	
	return events;
}
