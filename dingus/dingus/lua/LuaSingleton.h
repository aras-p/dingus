// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LUA_SINGLETON_H
#define __LUA_SINGLETON_H

#include "LuaWrapper.h"
#include "../utils/Singleton.h"

namespace dingus {


class CLuaSingleton : public CLuaWrapper, public CSingleton<CLuaSingleton> {
public:
	static void init( const std::string& predir ) {
		CLuaSingleton* lua = new CLuaSingleton( predir );
		assert( lua );
		assignInstance( *lua );
	}

private:
	CLuaSingleton( const std::string& predir ) : CLuaWrapper(predir) { }
	
	static CLuaSingleton* createInstance() { ASSERT_FAIL_MSG( "must be initialized first" ); return NULL; }
	static void deleteInstance( CLuaSingleton& instance ) { delete &instance; }
	friend CSingleton<CLuaSingleton>;
};


}; // namespace


#endif
