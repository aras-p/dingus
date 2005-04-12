// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RANDOM_H__
#define __RANDOM_H__

namespace dingus {

/**
 *  Returns random number [0..interv)
 */
	/*
inline float randf( float interv = 1.0 )
{
	return (float)rand() / RAND_MAX * interv;
};
*/

/**
 *  Returns random number [from..to)
 */
	/*
inline float randf( float from, float to )
{
	return randf( to - from ) + from;
};
*/


// --------------------------------------------------------------------------

/**
 *  Random number generator based on Mersenne Twister.
 *  http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 */
class CRandom {
public:
	CRandom();

	/** Initialize with a seed. */
	void seed( unsigned int s );
	/** Initialize with a large seed or state array. */
	void seedByArray( const unsigned int keys[], int keyCount );

	/** Generates a random integer on [0,2^32) interval. */
	unsigned int getUInt();

	/** Generates a random integer on [-2^31,2^31) interval. */
	int getInt() { return (int)getUInt(); }

	/** Generates a random integer on [low,hi) interval. */
	int getInt( int low, int hi );

	/** Generates a floating point number on [0.0, 1.0) interval. */
	double getDouble() { return getUInt()*(1.0/4294967296.0); /* divided by 2^32 */ }

	/** Generates a floating point number on [0.0, 1.0) interval. */
	float getFloat() { return float( getDouble() ); }

	/** Generates a floating point number on [0.0, interv) interval. */
	float getFloat( float interv ) { return getFloat() * interv; }

	/** Generates a floating point number on [low, high) interval. */
	float getFloat( float low, float hi ) { return getFloat() * (hi - low) + low; }

	/** Generates a random bool. */
	bool	getBool() { return getUInt() >= 0x80000000UL; }

private:
	enum { N = 624, M = 397 };	// Period parameters.
	unsigned int	mState[N];	// The state vector.
	int				mIndex;		// mIndex>=N means state has to be re-initialized.
};

/// Global random generator.
extern	CRandom	gRandom;



// --------------------------------------------------------------------------


/**
 *  Very fast random bytes and words.
 *  NOT as good random properties as CRandom.
 *  Each random is about 11 clocks.
 */
class CRandomFast {
public:
	CRandomFast();
	
	void	seed( unsigned int s );
	unsigned char	getByte();
	unsigned short	getWord();
	bool			getBool();

private:
	unsigned int	mLastRand;
};

// -- implementation ------------------

inline CRandomFast::CRandomFast()
:	mLastRand(0xA2A9) // a prime
{
}
inline void CRandomFast::seed( unsigned int s )
{
	// xor by something so that we have an ok value even when 0 is passed in
	mLastRand = s ^ 0xA2A9;
}
inline unsigned char CRandomFast::getByte()
{
	mLastRand = mLastRand * 65539 + 3;
	return (unsigned char)(mLastRand >> 24);
}
inline unsigned short CRandomFast::getWord()
{
	mLastRand = mLastRand * 65539 + 3;
	return (unsigned short)(mLastRand >> 16);
}
inline bool CRandomFast::getBool()
{
	mLastRand = mLastRand * 65539 + 3;
	return mLastRand >= 0x80000000UL;
}


}; // namespace

#endif 
