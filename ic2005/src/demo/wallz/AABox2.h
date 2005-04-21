#ifndef __MATH_AABOX_2D_H
#define __MATH_AABOX_2D_H

#include <dingus/math/Vector2.h>
#include <dingus/math/Constants.h>

namespace dingus {


/**
 *  Axis aligned 2D box.
 *
 *  Essentially two SVector2's - one for minimum, one for maximum.
 */
class CAABox2 {
public:
	/// Initializes to "none" AABB - minimum to big numbers, maximum to big negative numbers.
	CAABox2()
	:	mMin(DINGUS_BIG_FLOAT,DINGUS_BIG_FLOAT),
		mMax(-DINGUS_BIG_FLOAT,-DINGUS_BIG_FLOAT) { }

	/// Initializes to "none" AABB - minimum to big numbers, maximum to big negative numbers.
	void setNull() {
		mMin.x = mMin.y = DINGUS_BIG_FLOAT;
		mMax.x = mMax.y = -DINGUS_BIG_FLOAT;
	}

	/// Gets minimum vector.
	const SVector2& getMin() const { return mMin; }
	/// Gets minimum vector.
	SVector2& getMin() { return mMin; }
	/// Gets maximum vector.
	const SVector2& getMax() const { return mMax; }
	/// Gets maximum vector.
	SVector2& getMax() { return mMax; }

	/// Gets center vector.
	SVector2 getCenter() const { return (mMax+mMin)*0.5f; }
	/// Gets size vector.
	SVector2 getSize() const { return mMax-mMin; }

	/// Extends a box to include point
	void	extend( const SVector2& p ) {
		if( p.x < mMin.x ) mMin.x = p.x;
		if( p.y < mMin.y ) mMin.y = p.y;
		if( p.x > mMax.x ) mMax.x = p.x;
		if( p.y > mMax.y ) mMax.y = p.y;
	}
	/// Extends a box to include point and some more space
	void	extend( const SVector2& p, float more ) {
		if( p.x-more < mMin.x ) mMin.x = p.x-more;
		if( p.y-more < mMin.y ) mMin.y = p.y-more;
		if( p.x+more > mMax.x ) mMax.x = p.x+more;
		if( p.y+more > mMax.y ) mMax.y = p.y+more;
	}
	/// Extends a box to include other box
	void	extend( const CAABox2& b ) {
		extend( b.getMin() );
		extend( b.getMax() );
	}

	/// Tests if a point is inside this box
	bool isInside( const SVector2& p ) const {
		return
			p.x>=mMin.x && p.x<=mMax.x &&
			p.y>=mMin.y && p.y<=mMax.y;
	}

private:
	SVector2	mMin;
	SVector2	mMax;
};


}; // namespace


#endif
