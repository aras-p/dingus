// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LUA_WRAPPER_H
#define __LUA_WRAPPER_H


extern "C" {
#include "lua.h"
};

namespace dingus {

class CLuaValue;


/**
 *  Wrapper for LUA language API.
 */
class CLuaWrapper {
public:
    CLuaWrapper( const std::string& basePath );
    ~CLuaWrapper();

	/** Returns global variable. */
	CLuaValue	getGlobal( const char* name );
	/** Returns global variable. */
	CLuaValue	getGlobal( const std::string& name );
	/** Returns globals table. */
	CLuaValue	getGlobals();
	/** Clears global. */
	void		clearGlobal( const char* name );
	/** Clears global. */
	void		clearGlobal( const std::string& name );

	/** Returns Lua error code. */
	int	doFile( const char* fileName, bool addBasePath );
	/** Returns Lua error code. */
	int	doString( const char* str );

	/** Discards last obtained value. */
	void discard() { lua_pop(mState,1); }
	

	/** Was last parse errorneous? */
	bool isError() const { return mError!=0; };
	const char* getError() const;

	lua_State* getState() const { return mState; }

private:
	/** Lua state. */
	lua_State*	mState;
	/** Errors during last parse. */
	int mError;
	/** Base path for Lua files. */
	std::string		mBasePath;
};



inline int CLuaWrapper::doFile( const char* fileName, bool addBasePath )
{
	assert( fileName!=NULL );
	return lua_dofile(mState, addBasePath ? ((mBasePath+fileName).c_str()) : fileName );
}

inline int CLuaWrapper::doString( const char* str )
{
	assert( str!=NULL );
	return lua_dostring(mState, str);
}

}; // namespace

#include "LuaValue.h"

namespace dingus {
	inline CLuaValue CLuaWrapper::getGlobal( const char* name )
	{
		lua_getglobal(mState,name);
		return CLuaValue( *this,lua_gettop(mState) );
	}
	inline CLuaValue CLuaWrapper::getGlobal( const std::string& name )
	{
		return getGlobal( name.c_str() );
	}
	inline CLuaValue CLuaWrapper::getGlobals()
	{
		lua_getglobals(mState);
		return CLuaValue( *this,lua_gettop(mState) );
	}
	inline void CLuaWrapper::clearGlobal( const char* name )
	{
		lua_pushnil(mState);
		lua_setglobal(mState,name);
	}
	inline void CLuaWrapper::clearGlobal( const std::string& name )
	{
		clearGlobal( name.c_str() );
	}
}; // namespace

#endif
