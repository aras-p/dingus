// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TIMER_H__
#define __TIMER_H__


namespace dingus {

// --------------------------------------------------------------------------

/**
 *  Quantized time value.
 *
 *  Unlike floats/doubles, doesn't lose precision. Internal 64 bit
 *  representation provides lots of range and precision: quantized at million
 *  in a second, provides ~290 thousand years.
 */
struct time_value {
public:
	/// Quantize into one million-th of a second.
	enum { FREQ = 1000*1000 };
public:
	typedef __int64 value_type;
public:
	explicit time_value( value_type v = 0 ) : value(v) { }

	static time_value fromsec( float seconds ) { return time_value( value_type(((double)seconds)*FREQ) ); }
	float tosec() const { return (float)value / FREQ; }

	void	zero() { value = 0; }

	void operator+=( const time_value& t ) { value += t.value; }
	void operator-=( const time_value& t ) { value -= t.value; }

public:
	value_type value;
};

inline time_value operator+( const time_value& a, const time_value& b ) {
	return time_value( a.value + b.value );
};
inline time_value operator-( const time_value& a, const time_value& b ) {
	return time_value( a.value - b.value );
};


// --------------------------------------------------------------------------

/**
 *  Abstract time source.
 */
class ITimeSource : public boost::noncopyable {
public:
	virtual ~ITimeSource() = 0 { }
	
	virtual time_value	getTime() const = 0;
	virtual time_value	getDeltaTime() const = 0;
	virtual double		getTimeS() const = 0;
	virtual float		getDeltaTimeS() const = 0;
};


// --------------------------------------------------------------------------


/**
 *  System time timer class.
 */
class CSystemTimer : public ITimeSource {
public:
	static CSystemTimer& getInstance() { return mInstance; }

	void	reset() { mTime.value = mDeltaTime.value = 0; setTimesS(); };
	void	start() { mStopped = false; }
	void	stop() { mStopped = true; mDeltaTime.value = 0; setTimesS(); }
	void	advance( time_value v ) { mTime += v; mDeltaTime += v; setTimesS(); }

	/// Update timer
	void	update();
	/// Update with fixed step. For debugging purposes.
	void	updateFixed( time_value dt );

	// ITimeSource
	virtual time_value	getTime() const { return mTime; }
	virtual time_value	getDeltaTime() const { return mDeltaTime; }
	virtual double		getTimeS() const { return mTimeS; }
	virtual float		getDeltaTimeS() const { return mDeltaTimeS; }

private:
	CSystemTimer();
	time_value fetchTime() const;
	void	setTimesS() { mTimeS = mTime.tosec(); mDeltaTimeS = mDeltaTime.tosec(); }

private:
	double		mFreqDivMyFreq;
	bool		mUsingQPF; ///< QPF or timeGetTime
	bool		mStopped;

	/// App time (eg. paused periods don't count)
	time_value	mTime;
	/// Time between last updates
	time_value	mDeltaTime;
	/// Absolute time
	time_value	mAbsTime;
	/// Time pre-converted into seconds
	double		mTimeS;
	/// Delta time pre-converted into seconds
	float		mDeltaTimeS;

	static CSystemTimer	mInstance;
};


// --------------------------------------------------------------------------

/**
 *  Arbitrary time timer.
 */
class CTimer : public ITimeSource {
public:
	CTimer() : mTime(0), mDeltaTime(0) { setTimesS(); }

	void	update( const time_value& dt ) {
		mTime += dt;
		mDeltaTime = dt;
		setTimesS();
	}

	// ITimeSource
	virtual time_value	getTime() const { return mTime; }
	virtual time_value	getDeltaTime() const { return mDeltaTime; }
	virtual double		getTimeS() const { return mTimeS; }
	virtual float		getDeltaTimeS() const { return mDeltaTimeS; }

private:
	void	setTimesS() { mTimeS = mTime.tosec(); mDeltaTimeS = mDeltaTime.tosec(); }

private:
	/// Time
	time_value	mTime;
	/// Time between last updates
	time_value	mDeltaTime;
	/// Time pre-converted into seconds
	double		mTimeS;
	/// Delta time pre-converted into seconds
	float		mDeltaTimeS;
};



}; // namespace dingus


#endif
