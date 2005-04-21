#include "stdafx.h"
#include "Triangulate.h"


namespace triangulator {

	static const float EPSILON = 0.0000000001f;

	static bool snip( const TVec2Vector& vb, int u,int v,int w,int n, const int *V );

};


float triangulator::area( const TVec2Vector& vb, const TIntVector& ib )
{
	int n = ib.size();
	float A = 0.0f;
	for( int p=n-1,q=0; q<n; p=q++ ) {
		A += vb[ib[p]].x * vb[ib[q]].y - vb[ib[q]].x * vb[ib[p]].y;
	}
	return A*0.5f;
}

bool triangulator::insideTri(
							float Ax, float Ay,
							float Bx, float By,
							float Cx, float Cy,
							float Px, float Py )
								 
{
	float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;
	
	ax = Cx - Bx;  ay = Cy - By;
	bx = Ax - Cx;  by = Ay - Cy;
	cx = Bx - Ax;  cy = By - Ay;
	apx= Px - Ax;  apy= Py - Ay;
	bpx= Px - Bx;  bpy= Py - By;
	cpx= Px - Cx;  cpy= Py - Cy;
	
	aCROSSbp = ax*bpy - ay*bpx;
	cCROSSap = cx*apy - cy*apx;
	bCROSScp = bx*cpy - by*cpx;
	
	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}

bool triangulator::snip( const TVec2Vector& vb, int u,int v,int w,int n, const int *V )
{
	int p;
	float Ax, Ay, Bx, By, Cx, Cy, Px, Py;
	
	Ax = vb[V[u]].x;
	Ay = vb[V[u]].y;
	
	Bx = vb[V[v]].x;
	By = vb[V[v]].y;
	
	Cx = vb[V[w]].x;
	Cy = vb[V[w]].y;
	
	if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;
	
	for( p=0;p<n;p++ ) {
		if( (p == u) || (p == v) || (p == w) ) continue;
		Px = vb[V[p]].x;
		Py = vb[V[p]].y;
		if( insideTri(Ax,Ay,Bx,By,Cx,Cy,Px,Py) )
			return false;
	}
	
	return true;
}

bool triangulator::process( const TVec2Vector& vb, const TIntVector& ib, TIntVector& res )
{
	// allocate and initialize list of vertices in polygon
	
	int n = ib.size();
	if( n < 3 )
		return false;
	
	int *V = new int[n];
	
	// we want a counter-clockwise polygon in V
	if ( 0.0f < area(vb,ib) ) {
		for( int v=0; v<n; ++v )
			V[v] = ib[v];
	} else {
		for( int v=0; v<n; ++v )
			V[v] = ib[(n-1)-v];
	}
			
	int nv = n;
	
	//	remove nv-2 vertices, creating 1 triangle every time
	int count = 2*nv;	// error detection
	
	for( int m=0, v=nv-1; nv>2; ) {
		// if we loop, it is probably a non-simple polygon
		if( 0 >= (count--) ) {
			// ERROR - probable bad polygon!
			return false;
		}
		
		// three consecutive vertices in current polygon, <u,v,w>
		int u = v  ; if (nv <= u) u = 0;	// previous
		v = u+1; if (nv <= v) v = 0;		// new v
		int w = v+1; if (nv <= w) w = 0;	// next
		
		if ( snip(vb,u,v,w,nv,V) ) {
			int a,b,c,s,t;
			
			// true names of the vertices
			a = V[u]; b = V[v]; c = V[w];
			
			// output triangle
			res.push_back( a );
			res.push_back( b );
			res.push_back( c );
			
			m++;
			
			// remove v from remaining polygon
			for(s=v,t=v+1;t<nv;s++,t++)
				V[s] = V[t]; nv--;
			
			// reset error detection counter
			count = 2*nv;
		}
	}

	delete[] V;

	return true;
}


/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "triangulate.h"

void main(int argc,char **argv)
{
	
	// Create a pretty complicated little contour by pushing them onto
	// an stl vector.
	
	TVec2Vector a;
	
	a.push_back( SVector2(0,6));
	a.push_back( SVector2(0,0));
	a.push_back( SVector2(3,0));
	a.push_back( SVector2(4,1));
	a.push_back( SVector2(6,1));
	a.push_back( SVector2(8,0));
	a.push_back( SVector2(12,0));
	a.push_back( SVector2(13,2));
	a.push_back( SVector2(8,2));
	a.push_back( SVector2(8,4));
	a.push_back( SVector2(11,4));
	a.push_back( SVector2(11,6));
	a.push_back( SVector2(6,6));
	a.push_back( SVector2(4,3));
	a.push_back( SVector2(2,6));
	
	// allocate an STL vector to hold the answer.
	
	TVec2Vector res;
	
	//	Invoke the triangulator to triangulate this polygon.
	triangulator::process(a,res);
	
	// print out the results.
	int tcount = res.size()/3;
	
	for (int i=0; i<tcount; i++)
	{
		const SVector2 &p1 = res[i*3+0];
		const SVector2 &p2 = res[i*3+1];
		const SVector2 &p3 = res[i*3+2];
		printf("Triangle %d => (%0.0f,%0.0f) (%0.0f,%0.0f) (%0.0f,%0.0f)\n",i+1,p1.x,p1.y,p2.x,p2.y,p3.x,p3.y);
	}
	
}
*/