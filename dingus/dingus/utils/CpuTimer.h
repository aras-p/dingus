#ifndef __CPU_TIMER_H
#define __CPU_TIMER_H
#pragma once

namespace dingus {


/**
 *  Fast and accurate timer based on CPU ticks.
 *
 *  However, the actual CPU speed varies on speed-step CPUs,
 *  so don't use it in production code (but it's great for timing
 *  when developing, and you're sure the CPU isn't speed-stepped).
 */
namespace cputimer
{
	typedef __int64 ticks_type;
	
	/// Returns the time in CPU ticks.
	ticks_type	ticks();

	/// Returns the time in seconds.
	double		seconds();

	/// Returns CPU speed in MHz.
	int			mhz();

	/// Returns seconds per CPU tick.
	double		secsPerTick();


	class debug_interval : public boost::noncopyable {
	public:
		debug_interval( const char* name );
		~debug_interval();
	private:
		ticks_type	mStartTicks;
		const char*	mName;
	};
};

}; // namespace dingus


#endif
