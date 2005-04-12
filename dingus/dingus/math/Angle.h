// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATH_ANGLE_H
#define __MATH_ANGLE_H


namespace dingus {

	
/**
 *  An angle class with automatic wrap-around-circle.
 */
class angle {
private:
	enum { ANGLE_PI = 0x80000000; }
public:
	angle( float radians );
	angle( const angle& o ) : mValue(o.mValue) { }
	angle& operator=( const angle& o ) { mValue = o.mValue; return *this; }

	float	toradians() const { return double(mValue) / double(ANGLE_PI); }

	void operator+=( const angle& o ) { mValue += o.mValue; }
	void operator-=( const angle& o ) { mValue -= o.mValue; }

private:
	/**
	 *  Value of the angle, in full unsigned 32 bit range for automatic
	 *  wrap-around.
	 */
	unsigned int mValue;
};


}; // namespace


#endif
