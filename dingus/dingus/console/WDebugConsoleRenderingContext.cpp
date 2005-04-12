// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "WDebugConsoleRenderingContext.h"

using namespace dingus;


void CWDebugConsoleRenderingContext::write( const std::string& message )
{
	OutputDebugString( message.c_str() );
}
