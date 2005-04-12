// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Timer.h"

#include <mmsystem.h>

using namespace dingus;

//---------------------------------------------------------------------------

CSystemTimer CSystemTimer::mInstance;


CSystemTimer::CSystemTimer()
:	mTime(0), mDeltaTime(0)
{
	// Use QueryPerformanceFrequency() to get frequency of timer. If QPF
	// is not supported, we will timeGetTime() which returns milliseconds.
	LARGE_INTEGER ticksPerSec;
	mUsingQPF = QueryPerformanceFrequency( &ticksPerSec ) ? true : false;
	if( mUsingQPF )
		mFreqDivMyFreq = (double)time_value::FREQ / (double)ticksPerSec.QuadPart;
	else
		mFreqDivMyFreq = (double)time_value::FREQ / 1000.0;

	// Fetch absolute time
	mAbsTime = fetchTime();

	setTimesS();
}


time_value CSystemTimer::fetchTime() const
{
	// get current time
	if( mUsingQPF ) {
		LARGE_INTEGER qpft;
		QueryPerformanceCounter( &qpft );
		return time_value( time_value::value_type( (double)qpft.QuadPart * mFreqDivMyFreq ) );
	} else {
		DWORD tmt = timeGetTime();
		return time_value( time_value::value_type( (double)tmt * mFreqDivMyFreq ) );
	}
}

void CSystemTimer::update()
{
	// get current time
	time_value now = fetchTime();

	// delta
	mDeltaTime = now - mAbsTime;

	// absolute time
	mAbsTime = now;

	// time - if we're paused, don't advance time and set delta to zero
	if( mStopped ) {
		mDeltaTime.value = 0;
	} else {
		mTime += mDeltaTime;
	}

	// preconvert t and dt into seconds for convenience and (possibly) efficiency
	setTimesS();
}

void CSystemTimer::updateFixed( time_value dt )
{
	mDeltaTime = dt;
	mAbsTime += dt;

	// time - if we're paused, don't advance time and set delta to zero
	if( mStopped ) {
		mDeltaTime.value = 0;
	} else {
		mTime += mDeltaTime;
	}

	// preconvert t and dt into seconds for convenience and (possibly) efficiency
	setTimesS();
}
