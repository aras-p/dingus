// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "LuaWrapper.h"

using namespace dingus;



CLuaWrapper::CLuaWrapper( const std::string& basePath )
:	mError( 0 ), mState( NULL ), mBasePath(basePath)
{
	mState = lua_open( 0 );
	assert( mState!=NULL );
}

CLuaWrapper::~CLuaWrapper()
{
	lua_close( mState );
}


const char* CLuaWrapper::getError() const
{
	return strerror( mError );
}
