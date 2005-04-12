// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "LuaValue.h"

using namespace dingus;



std::string CLuaValue::getString() const 
{
	int n = lua_strlen(mLua->getState(),mIndex);
	if( n < 1 )
		return "";
	return std::string(lua_tostring(mLua->getState(),mIndex),n);
};

CLuaValue CLuaValue::getElement( const char* key ) const
{
	lua_pushstring( mLua->getState(), key );
	lua_rawget( mLua->getState(), mIndex );
	return CLuaValue( *mLua, lua_gettop(mLua->getState()) );
}
CLuaValue CLuaValue::getElement( double key ) const
{
	lua_pushnumber( mLua->getState(), key );
	lua_rawget( mLua->getState(), mIndex );
	return CLuaValue( *mLua, lua_gettop(mLua->getState()) );
}
