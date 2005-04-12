// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TRIANGLE_H
#define __TRIANGLE_H


#include "Vector3.h"
#include "Line3.h"
#include "Plane.h"


namespace dingus {
	

/**
 *  Triangle is defined with one vertex and two edge vectors.
 *
 *  All the triangle points can be obtained with Barycentric coordinates:
 *  tri(s,t) = base + s*edge0 + t*edge1, where s,t,s+t are in [0..1].
 */
class STriangle {
public:
	STriangle() { }
	STriangle( const SVector3& v0, const SVector3& v1, const SVector3& v2 )
		: base(v0), edge0(v1-v0), edge1(v2-v0) { }
	
	void set( const SVector3& v0, const SVector3& v1, const SVector3& v2 ) {
		base = v0; edge0 = v1-v0; edge1 = v2-v0;
	}
	
	/** Gets unit length normal of the triangle (edge0 cross edge1). */
	SVector3 getNormal() const { return edge0.cross(edge1).getNormalized(); }

	/** Gets unnormalized normal of the triangle (edge0 cross edge1). */
	SVector3 getNormalNoNorm() const { return edge0.cross(edge1); }
	
	/** Gets the midpoint of the triangle. */
	SVector3 getMidpoint() const { return base + (edge0+edge1) * (1.0f/3.0f); }

	/** Constructs a plane from a triangle. */
	SPlane getPlane() const { return SPlane(base,base+edge0,base+edge1); }
	
	/** Gets point 0 (base). */
	SVector3 const& getPoint0() const { return base; }
	/** Gets point 1 (base+edge0). */
	SVector3 getPoint1() const { return base+edge0; }
	/** Gets point 2 (base+edge1). */
	SVector3 getPoint2() const { return base+edge1; }

	/**
	 *  Intersect a line with a triangle.
	 *  @param ipos Interpolated position of intersection on a line.
	 *  @return false if no intersection or hits backside.
	 */
	bool intersect( const SLine3& line, float& ipos ) const;
	
	/**
	 *  Intersect a line with a triangle.
	 *  @param ipos Interpolated position of intersection on a line.
	 *  @return false if no intersection.
	 */
	bool intersect2Sided( const SLine3& line, float& ipos ) const;

public:
	SVector3	base;
	SVector3	edge0;
	SVector3	edge1;
};


}; // namespace

#endif
