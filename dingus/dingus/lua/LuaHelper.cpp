// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "LuaHelper.h"

using namespace dingus;

std::string CLuaHelper::getString( CLuaValue& val, const char* name )
{
	std::string s = val.getElement( name ).getString();
	val.getLua().discard();
	return s;
}

double CLuaHelper::getNumber( CLuaValue& val, const char* name )
{
	double d = val.getElement( name ).getNumber();
	val.getLua().discard();
	return d;
}

SVector3 CLuaHelper::getVector3( CLuaValue& val, const char* name )
{
	CLuaWrapper& lua = val.getLua();
	CLuaValue luaVec = val.getElement(name);
	SVector3 vec;
	if( luaVec.isTable() ) {
		vec.x = (float)luaVec.getElement(1).getNumber();
		vec.y = (float)luaVec.getElement(2).getNumber();
		vec.z = (float)luaVec.getElement(3).getNumber();
		lua.discard(); lua.discard(); lua.discard();
	} else {
		vec.set(0,0,0);
	}
	lua.discard();
	return vec;
}

SQuaternion CLuaHelper::getQuaternion( CLuaValue& val, const char* name )
{
	CLuaWrapper& lua = val.getLua();
	CLuaValue luaQ = val.getElement(name);
	SQuaternion q;
	if( luaQ.isTable() ) {
		q.x = (float)luaQ.getElement(1).getNumber();
		q.y = (float)luaQ.getElement(2).getNumber();
		q.z = (float)luaQ.getElement(3).getNumber();
		q.w = (float)luaQ.getElement(4).getNumber();
		lua.discard(); lua.discard(); lua.discard(); lua.discard();
	} else {
		q = SQuaternion(0,0,0,1);
	}
	lua.discard();
	return q;
}

void CLuaHelper::getMatrix3x3( CLuaValue& val, const char* name, SMatrix4x4& m )
{
	CLuaWrapper& lua = val.getLua();
	CLuaValue luaM = val.getElement(name);
	if( luaM.isTable() ) {
		m._11 = (float)luaM.getElement(1).getNumber();
		m._12 = (float)luaM.getElement(2).getNumber();
		m._13 = (float)luaM.getElement(3).getNumber();
		m._21 = (float)luaM.getElement(4).getNumber();
		m._22 = (float)luaM.getElement(5).getNumber();
		m._23 = (float)luaM.getElement(6).getNumber();
		m._31 = (float)luaM.getElement(7).getNumber();
		m._32 = (float)luaM.getElement(8).getNumber();
		m._33 = (float)luaM.getElement(9).getNumber();
		lua.discard(); lua.discard(); lua.discard();
		lua.discard(); lua.discard(); lua.discard();
		lua.discard(); lua.discard(); lua.discard();
	} else {
		m.getAxisX().set(0,0,0);
		m.getAxisY().set(0,0,0);
		m.getAxisZ().set(0,0,0);
	}
	lua.discard();
}
