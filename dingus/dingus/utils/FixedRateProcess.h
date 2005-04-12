// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FIXED_RATE_PROCESS_H
#define __FIXED_RATE_PROCESS_H

namespace dingus {


class CFixedRateProcess {
public:
	CFixedRateProcess( float performFreq = 60.0f, int maxPerformsAtOnce = 20 ) : mTimeSinceLastUpdate(0), mMaxPerformsAtOnce(maxPerformsAtOnce) { setPerformFreq(performFreq); }
	virtual ~CFixedRateProcess() = 0 { }

	void perform();

	void	setPerformFreq( float freq ) { mPerformFreq = freq; mPerformDT = 1.0f / freq; }
	float	getPerformFreq() const { return mPerformFreq; }
	float	getPerformDT() const { return mPerformDT; }

protected:
	virtual void performProcess() = 0;

private:
	float	mPerformFreq;
	float	mPerformDT;
	float	mTimeSinceLastUpdate;
	int		mMaxPerformsAtOnce;
};

};


#endif
