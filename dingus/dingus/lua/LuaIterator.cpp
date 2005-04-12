// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "LuaIterator.h"

using namespace dingus;

CLuaArrayIterator::CLuaArrayIterator( CLuaValue& val )
:	mLua(&val.getLua()),
	mValue(&val),
	mFetchedElem(val.isTable() ? val.getElement(1) : CLuaValue() ),
	mIndex(1)
{
}

CLuaArrayIterator::~CLuaArrayIterator()
{
	if( mValue->isTable() )
		mLua->discard();
}

bool CLuaArrayIterator::hasNext() const
{
	if( !mValue->isTable() )
		return false;
	bool ok = !mValue->getElement(mIndex).isNil();
	mLua->discard();
	return ok;
}

CLuaValue& CLuaArrayIterator::next()
{
	assert( hasNext() );
	// first elem?
	if( mIndex == 1 ) {
		++mIndex;
	} else {
		// discard last element fetched
		mLua->discard();
		// fetch new element
		mFetchedElem = mValue->getElement(mIndex);
		++mIndex;
	}
	return mFetchedElem;
}
