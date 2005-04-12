// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "FixedRateProcess.h"

using namespace dingus;


void CFixedRateProcess::perform()
{
	float t = CSystemTimer::getInstance().getDeltaTimeS() + mTimeSinceLastUpdate;
	int count = 0;
	while( t >= mPerformDT ) {
		if( count < mMaxPerformsAtOnce )
			performProcess();
		t -= mPerformDT;
		++count;
	}
	mTimeSinceLastUpdate = t;
}
