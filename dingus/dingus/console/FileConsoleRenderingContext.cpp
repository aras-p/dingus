// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "FileConsoleRenderingContext.h"
#include <time.h>

using namespace dingus;

//---------------------------------------------------------------------------
// CFileConsoleRenderingContext
//---------------------------------------------------------------------------

CFileConsoleRenderingContext::CFileConsoleRenderingContext( const std::string& fileName )
:	mFile( fileName.c_str() )
{
}

CFileConsoleRenderingContext::~CFileConsoleRenderingContext()
{
}

void CFileConsoleRenderingContext::write( const std::string& message )
{
	mFile << message;
	mFile.flush();
}

