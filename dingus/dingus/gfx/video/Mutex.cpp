// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Mutex.h"

using namespace dingus;

//-----------------------------------------------------------------------------

CMutex::CMutex()
{
	InitializeCriticalSection( &mCritSec );
}
CMutex::~CMutex()
{
	DeleteCriticalSection( &mCritSec );
}

void CMutex::lock()
{
	EnterCriticalSection( &mCritSec );
}

void CMutex::unlock()
{
	LeaveCriticalSection( &mCritSec );
}

long CMutex::incrementRC( long* value )
{
	return InterlockedIncrement( value );
}

long CMutex::decrementRC( long* value )
{
	return InterlockedDecrement( value );
}
