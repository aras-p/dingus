// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "../dxutils/D3DFont.h"

#include "D3DConsoleRenderingContext.h"
#include "../kernel/D3DDevice.h"

using namespace dingus;

//---------------------------------------------------------------------------
// CD3DTextBoxConsoleRenderingContext
//---------------------------------------------------------------------------

CD3DTextBoxConsoleRenderingContext::CD3DTextBoxConsoleRenderingContext( CD3DFont& font, int x, int y, D3DCOLOR color1, D3DCOLOR color2 )
:	mFont( font ),
	mX( x ),
	mY( y ),
	mColor1( color1 ), mColor2( color2 )
{
}

void CD3DTextBoxConsoleRenderingContext::write( const std::string& message )
{
	mBuffer += message;
}

void CD3DTextBoxConsoleRenderingContext::flush()
{
	mFont.drawText( (float)mX+1, (float)mY+1, mColor2, (char*)mBuffer.c_str() );
	mFont.drawText( (float)mX, (float)mY, mColor1, (char*)mBuffer.c_str() );
	mBuffer.erase();
	CD3DDevice::getInstance().resetCachedState();
}
