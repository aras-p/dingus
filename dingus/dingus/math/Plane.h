// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PLANE_H
#define __PLANE_H


#include "Vector3.h"
#include "Line3.h"

namespace dingus {



class SPlane : public D3DXPLANE {
public:
	SPlane() : D3DXPLANE(0,0,0,1) { };
	SPlane( float A, float B, float C, float D ) : D3DXPLANE(A,B,C,D) { };
	SPlane( const float* coeffs ) : D3DXPLANE(coeffs) { }
	SPlane( const SVector3& v0, const SVector3& v1, const SVector3& v2 ) { D3DXPlaneFromPoints(this,&v0,&v1,&v2); }
	SPlane( const SVector3& p, const SVector3& n ) { D3DXPlaneFromPointNormal(this,&p,&n); }

	void set( float A, float B, float C, float D ) { a=A;b=B;c=C;d=D; }
	void set( const SVector3& v0, const SVector3& v1, const SVector3& v2 ) { D3DXPlaneFromPoints(this,&v0,&v1,&v2); }
	
	/** Distance of point to plane. */
	float distance( const SVector3& v ) const { return D3DXPlaneDotCoord(this,&v); }

	SVector3 getNormal() const { return SVector3(a,b,c); }

	bool intersect( const SLine3& l, SVector3& res ) const {
		return D3DXPlaneIntersectLine(&res,this,&l.getStart(),&l.getEnd()) ? true : false;
	}

	bool intersect( const SPlane& p, SLine3& res ) const;

	/// get intersecting t of line with one sided SPlane
	//bool intersect(const SLine3& l, float& t) const;
};


/*
inline bool SPlane::intersect(const SLine3& l, float& t) const
{
	float f0 = this->a * l.b.x + this->b * l.b.y + this->c * l.b.z + this->d;
	float f1 = this->a * -l.m.x + this->b * -l.m.y + this->c * -l.m.z;
	if ((f1 < -0.0001f) || (f1 > 0.0001f)) {
		t = f0 / f1;
		return true;
	} else {
		return false;
	}
}
*/

inline bool SPlane::intersect(const SPlane& p, SLine3& res) const
{
	SVector3 n0 = getNormal();
	SVector3 n1 = p.getNormal();
	float n00 = n0.lengthSq();	// 0 dot 0
	float n01 = n0.dot(n1);		// 0 dot 1
	float n11 = n1.lengthSq();	// 1 dot 1
	float det = n00 * n11 - n01 * n01;
	const float tol = 1e-06f;
	if( fabsf(det) < tol ) {
		return false;
	} else {
		float invDet = 1.0f/det;
		float c0 = (n11 *   d - n01 * p.d) * invDet;
		float c1 = (n00 * p.d - n01 *   d) * invDet;
		res.vec = n0.cross(n1);
		res.pos = n0 * c0 + n1 * c1;
		return true;
	}
}



}; // namespace

#endif
