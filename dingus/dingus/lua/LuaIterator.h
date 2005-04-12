// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LUA_ITERATOR_H
#define __LUA_ITERATOR_H

#include "LuaValue.h"


namespace dingus {

class CLuaArrayIterator : public boost::noncopyable {
public:
	CLuaArrayIterator( CLuaValue& val );
	~CLuaArrayIterator();

	bool hasNext() const;
	
	CLuaValue& next();

	int getKey() const { return mIndex-1; } // key (index) of last fetched elem

private:
	CLuaWrapper*	mLua;
	CLuaValue*		mValue;
	CLuaValue		mFetchedElem;
	int				mIndex;
};


}; // namespace


#endif
