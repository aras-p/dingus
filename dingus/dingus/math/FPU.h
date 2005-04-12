// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

// Pretty much taken from OPCODE (www.codercorner.com) or nVidia's fastmath.cpp

#ifndef __FPU_CODE_H
#define __FPU_CODE_H

/** Integer representation of 1.0 value. */
#define DINGUS_FPU_ONE			0x3F800000

/** Sign bitmask of float value. */
#define DINGUS_FPU_SIGN_MASK	0x80000000

/** Unsigned integer representation of a float value. */
#define DINGUS_FPU_UIR(x)		((unsigned int&)(x))

/** Signed integer representation of a float value. */
#define DINGUS_FPU_SIR(x)		((int&)(x))

/** Absolute integer representation of a float value. */
#define DINGUS_FPU_AIR(x)		(DINGUS_FPU_UIR(x)&0x7fffffff)

/** Float representation of an integer value. */
#define DINGUS_FPU_FR(x)		((float&)(x))

/**
 *  Integer-based comparison of a floating point value.
 *  Don't use it blindly, it can be faster or slower than the FPU
 *  comparison, depends on the context.
 */
#define DINGUS_FPU_IS_NEGATIVE(x)	(DINGUS_FPU_UIR(x)&0x80000000)


namespace dingus {

/**
 *  Fast fabs for floating-point values.
 *  It just clears the sign bit.
 *  Don't use it blindy, it can be faster or slower than the FPU comparison,
 *  depends on the context.
 */
inline float fastFabsf( float x )
{
	unsigned int bits = DINGUS_FPU_UIR(x)&0x7fffffff;
	return DINGUS_FPU_FR(bits);
}


/** Computes 1 / sqrt(x). */
inline float invSqrt( float f )
{
	float x = f * 0.5f;
	unsigned int y = 0x5f3759df - ( DINGUS_FPU_UIR(f) >> 1 );
	// iteration...
	(float&)y  = (float&)y * ( 1.5f - ( x * (float&)y * (float&)y ) );
	// result
	return (float&)y;
}

/** Computes 1/x. */
inline float inv( float x )
{
    int i = 2 * DINGUS_FPU_ONE - DINGUS_FPU_SIR(x);
    float r = DINGUS_FPU_FR(i);
    return r * (2.0f - x * r);
}


/**
 *  Quick rounding of float to integer.
 *  6 cycles or so. Can be used for quick float->int conversion (eg. for
 *  positive numbers: int i = round( f - 0.5f )).
 */
inline int round( float a )
{
	int retval;
	__asm fld a
	__asm fistp retval
	return retval;
}



}; // namespace

#endif
