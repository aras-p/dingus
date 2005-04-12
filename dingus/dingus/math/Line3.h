// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __LINE_H
#define __LINE_H

#include "Vector3.h"

namespace dingus {
	
	
class SLine3 {
public:
	SLine3() { };
	SLine3( const SVector3& v0, const SVector3& v1 ) : pos(v0), vec(v1-v0) { };
	
	void set( const SVector3& v0, const SVector3& v1 ) {
		pos = v0;
		vec = v1-v0;
	}
	const SVector3&	getStart() const { return pos; }
	const SVector3&	getVector() const { return vec; }
	SVector3	getEnd() const { return (pos+vec); };
	float		length() const { return vec.length(); };
	float		lengthSq() const { return vec.lengthSq(); }

	/** Distance from point to line. */
	float distance( const SVector3& p ) const {
		SVector3 diff( p-pos );
		float l = vec.lengthSq();
		if( l > 1.0e-6f ) {
			float t = vec.dot(diff) / l;
			diff = diff - vec*t;
			return diff.length();
		} else {
			// line is really a point...
			SVector3 v( p-pos );
			return v.length();
		}
	};
	
	/** Returns parametric point projection onto this line. */
	float project( const SVector3& p ) const {
		SVector3 diff( p-pos );
		float l = vec.lengthSq();
		assert( l > 1.0e-6f );
		return vec.dot(diff) / l;
	}
	
	SVector3 interpolate( float t ) const { return pos + vec*t; }
	
public:
	/** Line start point. */
	SVector3	pos;
	/** Line vector (end point minus start point). */
	SVector3	vec;
};

}; // namespace
	
	
#endif
