// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LUA_VALUE_H
#define __LUA_VALUE_H

namespace dingus {

class CLuaWrapper;


class CLuaValue {
public:
	/// Constructs invalid value. Caution with this!
	CLuaValue() : mLua(0), mIndex(0) { }

	/** Is uninitialized? */
	bool	isInvalid() const { return mLua==0; }
	/** Is this value nil? */
	bool	isNil() const;
	/** Is this value a number? */
	bool	isNumber() const;
	/** Is this value a string? */
	bool	isString() const;
	/** Is this value a table? */
	bool	isTable() const;
	/** Is this value a function? */
	bool	isFunction() const;

	/** Return value of number-value. */
	double		getNumber() const;
	/** Return value of string-value. */
	std::string	getString() const;
	/** Return element of table-value. */
	CLuaValue	getElement( const std::string& key ) const { return getElement(key.c_str()); }
	/** Return element of table-value. */
	CLuaValue	getElement( const char* key ) const;
	/** Return element of table-value. */
	CLuaValue	getElement( double key ) const;

	const CLuaWrapper& getLua() const { return *mLua; }
	CLuaWrapper& getLua() { return *mLua; }

	/** Discards value. */
	void discard();

protected:
	CLuaValue( CLuaWrapper& lua, int index ) : mLua(&lua), mIndex(index) { }

private:
	friend class CLuaWrapper;
	
private:
	CLuaWrapper*	mLua;
	int				mIndex;
};


}; // namespace


#include "LuaWrapper.h"

namespace dingus {
	inline bool CLuaValue::isNil() const { return lua_isnil(mLua->getState(),mIndex); }
	inline bool	CLuaValue::isNumber() const { return lua_isnumber(mLua->getState(),mIndex)?true:false; }
	inline bool	CLuaValue::isString() const { return lua_isstring(mLua->getState(),mIndex)?true:false; }
	inline bool	CLuaValue::isTable() const { return lua_istable(mLua->getState(),mIndex); }
	inline bool	CLuaValue::isFunction() const { return lua_isfunction(mLua->getState(),mIndex); }
	inline double CLuaValue::getNumber() const { return lua_tonumber(mLua->getState(),mIndex); }
	inline void CLuaValue::discard() { mLua->discard(); }
}; // namespace



#endif
