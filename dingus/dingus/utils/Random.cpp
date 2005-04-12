#include "stdafx.h"
#include "Random.h"

using namespace dingus;


// --------------------------------------------------------------------------

CRandom	dingus::gRandom;


#define MATRIX_A 0x9908b0dfUL	// constant vector a
#define UPPER_MASK 0x80000000UL // most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // least significant r bits


CRandom::CRandom()
:	mIndex(N+1)
{
	seed(5489UL); // default seed
}

void CRandom::seed( unsigned int s )
{
	mState[0] = s & 0xffffffffUL;
	for( mIndex = 1; mIndex < N; ++mIndex ) {
		mState[mIndex] = 
			(1812433253UL * (mState[mIndex-1] ^ (mState[mIndex-1] >> 30)) + mIndex);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mState[]. 				   */
		/* 2002/01/09 modified by Makoto Matsumoto			   */
		mState[mIndex] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

void CRandom::seedByArray( const unsigned int keys[], int keyCount )
{
	seed( 19650218UL );
	int i = 1, j = 0;
	int k = (N>keyCount ? N : keyCount);
	for( ; k; k-- ) {
		mState[i] = (mState[i] ^ ((mState[i-1] ^ (mState[i-1] >> 30)) * 1664525UL))
			+ keys[j] + j; /* non linear */
		mState[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i++; j++;
		if (i>=N) { mState[0] = mState[N-1]; i=1; }
		if (j>=keyCount) j=0;
	}
	for (k=N-1; k; k--) {
		mState[i] = (mState[i] ^ ((mState[i-1] ^ (mState[i-1] >> 30)) * 1566083941UL))
			- i; /* non linear */
		mState[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i++;
		if (i>=N) { mState[0] = mState[N-1]; i=1; }
	}
	
	mState[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

unsigned int CRandom::getUInt()
{
	unsigned int y;
	static const unsigned int mag01[2]={0x0UL, MATRIX_A};

	// regenerate state?
	if( mIndex >= N ) {
		int kk;
		for (kk=0;kk<N-M;kk++) {
			y = (mState[kk]&UPPER_MASK)|(mState[kk+1]&LOWER_MASK);
			mState[kk] = mState[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mState[kk]&UPPER_MASK)|(mState[kk+1]&LOWER_MASK);
			mState[kk] = mState[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mState[N-1]&UPPER_MASK)|(mState[0]&LOWER_MASK);
		mState[N-1] = mState[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
		mIndex = 0;
	}
	
	y = mState[mIndex++];
	
	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);
	return y;
}

int	CRandom::getInt( int low, int hi )
{
	assert( hi > low );
	const double scale = double(hi - low) * getDouble();
	const double retDoub = double(low) + scale;
	const int    retInt  = int(retDoub);
	assert( retInt < retDoub );
	assert( retInt >= low && retInt < hi );
	return retInt;
}

