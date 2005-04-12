// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Triangle.h"

using namespace dingus;


bool STriangle::intersect( const SLine3& line, float& ipos ) const
{
	const float tol = 1e-04f;
	SVector3	norm = getNormalNoNorm();
	float		denom = norm.dot( line.vec );
	
	if( denom >= -tol )
		return false; // backface or parallel
	
	// compute line t so that it intersects triangle's plane
	SVector3 kDiff0(line.pos - base);
	float time = -(norm.dot(kDiff0)) / denom;
	if( time<-tol || time>(1.0f+tol) )
		return false; // outside line range
	
	// diff of intersection point and vertex of triangle.
	SVector3 kDiff1( kDiff0 + line.vec*time );
	
	// see if inside tri
	float fE00 = edge0.lengthSq();	// 0 dot 0
	float fE01 = edge0.dot( edge1 );// 0 dot 1
	float fE11 = edge1.lengthSq();	// 1 dot 1
	float fDet = fabsf( fE00*fE11-fE01*fE01 );
	float fR0  = edge0.dot( kDiff1 );
	float fR1  = edge1.dot( kDiff1 );
	
	float fS0 = fE11*fR0 - fE01*fR1;
	float fS1 = fE00*fR1 - fE01*fR0;
	
	if( (fS0>=-tol) && (fS1>=-tol) && (fS0+fS1<=fDet+tol) ) {
		ipos = time;
		return true; // inside
	} else {
		return false; // outside
	}
}


bool STriangle::intersect2Sided( const SLine3& line, float& ipos ) const
{
	
	const float tol = 1e-04f;
	SVector3	norm = getNormalNoNorm();
	float		denom = norm.dot( line.vec );
	float llenSq = line.lengthSq();
	float nLenSq = norm.lengthSq();
	
	if( denom*denom <= tol*llenSq*nLenSq )
		return false;
	
	// compute line t so that it intersects triangle's plane
	SVector3 kDiff0(line.pos - base);
	float time = -(norm.dot(kDiff0)) / denom;
	if( time<-tol || time>(1.0f+tol) )
		return false; // outside line range
	
	// diff of intersection point and vertex of triangle.
	SVector3 kDiff1( kDiff0 + line.vec*time );
	
	// see if inside tri
	float fE00 = edge0.lengthSq();	// 0 dot 0
	float fE01 = edge0.dot( edge1 );// 0 dot 1
	float fE11 = edge1.lengthSq();	// 1 dot 1
	float fDet = fabsf( fE00*fE11-fE01*fE01 );
	float fR0  = edge0.dot( kDiff1 );
	float fR1  = edge1.dot( kDiff1 );
	
	float fS0 = fE11*fR0 - fE01*fR1;
	float fS1 = fE00*fR1 - fE01*fR0;
	
	if( (fS0>=-tol) && (fS1>=-tol) && (fS0+fS1<=fDet+tol) ) {
		ipos = time;
		return true; // inside
	} else {
		return false; // outside
	}
}
