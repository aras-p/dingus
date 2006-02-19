#include "stdafx.h"
#include "Hull.h"
#include <dingus/math/Plane.h>
#include <dingus/math/AABox.h>

// --------------------------------------------------------------------------


// Given inverse of view*proj matrix, calculate 8 corners of the frustum
void CalculateFrustumCoords( SVector3 pts[8], const SMatrix4x4& invViewProj )
{
	float cube[8][3] = {
		{ -1, -1, 0 },
		{  1, -1, 0 },
		{ -1,  1, 0 },
		{  1,  1, 0 },
		{ -1, -1, 1 },
		{  1, -1, 1 },
		{ -1,  1, 1 },
		{  1,  1, 1 },
	};
	D3DXVec3TransformCoordArray( pts, sizeof(SVector3), (D3DXVECTOR3*)cube, sizeof(SVector3), &invViewProj, 8 );
}

void CalculateFrustumHull( HullObject& obj, const SVector3 p[8] )
{
	obj.f.resize( 6 );
	for( int i = 0; i < 6; ++i )
		obj.f[i].v.resize( 4 );

	// near
	obj.f[0].v[0] = p[0];
	obj.f[0].v[1] = p[1];
	obj.f[0].v[2] = p[3];
	obj.f[0].v[3] = p[2];
	
	// far
	obj.f[1].v[0] = p[4];
	obj.f[1].v[1] = p[5];
	obj.f[1].v[2] = p[7];
	obj.f[1].v[3] = p[6];

	// sides
	obj.f[2].v[0] = p[0];
	obj.f[2].v[1] = p[1];
	obj.f[2].v[2] = p[5];
	obj.f[2].v[3] = p[4];
	
	obj.f[3].v[0] = p[1];
	obj.f[3].v[1] = p[3];
	obj.f[3].v[2] = p[7];
	obj.f[3].v[3] = p[5];

	obj.f[4].v[0] = p[1];
	obj.f[4].v[1] = p[3];
	obj.f[4].v[2] = p[7];
	obj.f[4].v[3] = p[5];
	
	obj.f[4].v[0] = p[3];
	obj.f[4].v[1] = p[2];
	obj.f[4].v[2] = p[4];
	obj.f[4].v[3] = p[5];
}

void CalcAABBPlanes( SPlane p[6], const CAABox& b )
{
	p[0].set(  0, -1,  0, fabs(b.getMin().y) );
	p[1].set(  0,  1,  0, fabs(b.getMax().y) );
	p[2].set( -1,  0,  0, fabs(b.getMin().x) );
	p[3].set(  1,  0,  0, fabs(b.getMax().x) );
	p[4].set(  0,  0, -1, fabs(b.getMin().z) );
	p[5].set(  0,  0,  1, fabs(b.getMax().z) );
}


void ClipPolyByPlane( const HullFace& f, const SPlane& A, HullFace& polyOut, HullFace& interPts )
{
	int i;
	int fsize = f.v.size();
	if( fsize < 3 )
		return;

	bool* outside = new bool[f.v.size()];
	//for each point
	for( i = 0; i < fsize; ++i )
	{
		outside[i] = A.distance( f.v[i] ) > 0.0f;
	}
	for( i = 0; i < fsize; ++i )
	{
		int idNext = (i+1) % fsize;
		// both outside -> save none
		if(outside[i] && outside[idNext]) {
			continue;
		}
		// outside-inside -> save intersection and i+1
		if(outside[i])
		{
			SLine3 edge( f.v[i], f.v[idNext] );
			SVector3 inter;
			if( A.intersect( edge, inter ) ) {
				polyOut.v.push_back( inter );
				interPts.v.push_back( inter );
			}
			polyOut.v.push_back( f.v[idNext] );
			continue;
		}
		// inside-outside -> save intersection
		if(outside[idNext]) {
			SLine3 edge( f.v[i], f.v[idNext] );
			SVector3 inter;
			if( A.intersect( edge, inter ) ) {
				polyOut.v.push_back( inter );
				interPts.v.push_back( inter );
			}
			continue;
		}
		// both inside -> save point i+1
		polyOut.v.push_back( f.v[idNext] );
	}

	delete[] outside;
}

static double RelativeEpsilon( double a, double epsilon )
{
	return max( fabs(a*epsilon), epsilon );
}

static bool Alike( double a, double b, double epsilon )
{
	if( a == b )
		return true;
	double relEps = RelativeEpsilon( a, epsilon );
	return (a-relEps <= b) && (b <= a+relEps);
}



static bool AlikeVector3( const SVector3& a, const SVector3& b, const double epsilon) {
	return 
		Alike(a[0],b[0],epsilon) &&
		Alike(a[1],b[1],epsilon) &&
		Alike(a[2],b[2],epsilon);
}


int FindSamePoint( const HullFace& f, const SVector3& p )
{
	int n = f.v.size();
	for( int i = 0; i < n; ++i )
	{
		if( AlikeVector3( f.v[i], p, 0.001) )
		{
			return i;
		}
	}
	return -1;
}

int FindSamePointAndSwapWithLast( HullObject& inter, const SVector3& p )
{
	int i;
	if( inter.f.empty() )
		return -1;
	for( i = inter.f.size()-1; i >= 0; --i )
	{
		HullFace& f = inter.f[i];
		if( 2 == f.v.size() )
		{
			const int idx = FindSamePoint( f, p );
			if( 0 <= idx )
			{
				//swap with last
				f.v.swap( inter.f.back().v );
				return idx;
			}
		}
	}
	return -1;
}


void AppendIntersectionFaces( HullObject& obj, HullObject& inter )
{
	// you need at least 3 sides for a polygon
	if( 3 > inter.f.size() )
		return;

	int size = obj.f.size();
	int i;
	
	// compact inter: remove f.size != 2 from end on forward
	for( i = inter.f.size(); 0 < i; --i ) {
		if( 2 == inter.f[i-1].v.size() ) {
			break;
		}
	}
	inter.f.resize( i );
	// you need at least 3 sides for a polygon
	if( 3 > inter.f.size() )
		return;

	// append one poly in obj
	obj.f.push_back( HullFace() );
	HullFace& polyOut = obj.f.back();
	
	// we have line segments in each face of inter 
	// take last line segment as first and second point
	const HullFace* polyIn = &(inter.f.back());
	polyOut.v.push_back( polyIn->v[0] );
	polyOut.v.push_back( polyIn->v[1] );
	// remove last f from inter, because it is already saved
	inter.f.pop_back();

	// iterate over inter until their is no line segment left
	while( !inter.f.empty() )
	{
		// pointer on last point to compare
		SVector3 *lastPt = &(polyOut.v.back());
		// find same point in inter to continue polygon
		const int idx = FindSamePointAndSwapWithLast( inter, *lastPt );
		if(0 <= idx) {
			// last line segment
			polyIn = &(inter.f.back());
			// get the other point in this polygon and save into polyOut
			polyOut.v.push_back( polyIn->v[(idx+1)%2] );
		}
		// remove last f from inter, because it is already saved or degenerated
		inter.f.pop_back();
	}
	// last point can be deleted, because he is the same as the first (closes polygon)
	polyOut.v.pop_back();
}


void ClipHullByPlane( HullObject& obj, const SPlane& A )
{
	if( obj.f.empty() )
		return;

	const HullObject objIn( obj );
	HullObject inter;

	obj.f.clear();

	int nInFaces = objIn.f.size();
	for( int i = 0; i < nInFaces; ++i )
	{
		int nOutFaces = obj.f.size();
		obj.f.push_back( HullFace() );
		inter.f.push_back( HullFace() );
		ClipPolyByPlane( objIn.f[i], A, obj.f.back(), obj.f.back() );
		
		// if whole face was clipped away -> delete empty f
		if( obj.f.back().v.empty() ) {
			obj.f.pop_back();
			inter.f.pop_back();
		}
	}
	// add a polygon with all intersection points to close the object
	AppendIntersectionFaces( obj, inter );
}



void ClipHullByAABB( HullObject& obj, const CAABox& box )
{
	SPlane planes[6];
	CalcAABBPlanes( planes,box );
	for( int i = 0; i < 6; ++i )
	{
		ClipHullByPlane( obj, planes[i] );
	}
}


// Returns our "focused region of interest": frustum, clipped by scene bounds,
// extruded towards light and clipped by scene bounds again.
void CalculateFocusedLightHull( const SMatrix4x4& invViewProj, const SVector3& lightDir, const CAABox& sceneAABB )
{
	SVector3 viewFrustum[8];
	CalculateFrustumCoords( viewFrustum, invViewProj );

	HullObject frustumHull;
	CalculateFrustumHull( frustumHull, viewFrustum );

	ClipHullByAABB( frustumHull, sceneAABB );
}
