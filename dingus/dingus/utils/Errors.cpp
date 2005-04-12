// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Errors.h"

using namespace dingus;

EBaseError::EBaseError( const std::string& msg, const char* fileName, int lineNumber )
:	std::runtime_error( msg ), mFileName( fileName ), mLineNumber( lineNumber )
{
	char buf[100];
	itoa( lineNumber, buf, 10 );
	mWhereMsg = msg + ' ' + fileName + ':' + buf;
};
