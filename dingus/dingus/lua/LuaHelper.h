// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LUA_HELPER_H
#define __LUA_HELPER_H

#include "LuaValue.h"

#include "../math/Vector3.h"
#include "../math/Quaternion.h"
#include "../math/Matrix4x4.h"

namespace dingus {


class CLuaHelper {
public:
	static std::string getString( CLuaValue& val, const char* name );
	static double getNumber( CLuaValue& val, const char* name );
	static SVector3 getVector3( CLuaValue& val, const char* name );
	static SQuaternion getQuaternion( CLuaValue& val, const char* name );
	static void getMatrix3x3( CLuaValue& val, const char* name, SMatrix4x4& m );
};


}; // namespace

#endif
