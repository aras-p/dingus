#ifndef TRIANGULATE_H
#define TRIANGULATE_H

// COTD Entry submitted by John W. Ratcliff [jratcliff@verant.com]

// ** THIS IS A CODE SNIPPET WHICH WILL EFFICIEINTLY TRIANGULATE ANY
// ** POLYGON/CONTOUR (without holes) AS A STATIC CLASS.  THIS SNIPPET
// ** IS COMPRISED OF 3 FILES, TRIANGULATE.H, THE HEADER FILE FOR THE
// ** TRIANGULATE BASE CLASS, TRIANGULATE.CPP, THE IMPLEMENTATION OF
// ** THE TRIANGULATE BASE CLASS, AND TEST.CPP, A SMALL TEST PROGRAM
// ** DEMONSTRATING THE USAGE OF THE TRIANGULATOR.	THE TRIANGULATE
// ** BASE CLASS ALSO PROVIDES TWO USEFUL HELPER METHODS, ONE WHICH
// ** COMPUTES THE AREA OF A POLYGON, AND ANOTHER WHICH DOES AN EFFICENT
// ** POINT IN A TRIANGLE TEST.
// ** SUBMITTED BY JOHN W. RATCLIFF (jratcliff@verant.com) July 22, 2000


/*****************************************************************/
/** Static class to triangulate any contour/polygon efficiently **/
/** You should replace SVector2 with whatever your own Vector	**/
/** class might be.  Does not support polygons with holes.		**/
/** Uses STL vectors to represent a dynamic array of vertices.	**/
/** This code snippet was submitted to FlipCode.com by			**/
/** John W. Ratcliff (jratcliff@verant.com) on July 22, 2000	**/
/** I did not write the original code/algorithm for this		**/
/** this triangulator, in fact, I can't even remember where I	**/
/** found it in the first place.  However, I did rework it into **/
/** the following black-box static class so you can make easy	**/
/** use of it in your own code.  Simply replace SVector2 with	**/
/** whatever your own Vector implementation might be.			**/
/*****************************************************************/

#include <dingus/math/Vector2.h>

namespace triangulator {

typedef std::vector< SVector2 > TVec2Vector;
typedef std::vector< int > TIntVector;

/**
 *  Triangulate a contour/polygon, place results in STL vector
 *  as series of triangles (3 indices per triangle).
 */
bool process( const TVec2Vector& vb, const TIntVector& ib, TIntVector& res );

/**
 *  Compute area of a contour/polygon.
 */
float area( const TVec2Vector& vb, const TIntVector& ib );

/**
 *  Decide if point (Px,Py) is inside triangle defined by
 *  (Ax,Ay) (Bx,By) (Cx,Cy).
 */
bool insideTri(
	float Ax, float Ay,
	float Bx, float By,
	float Cx, float Cy,
	float Px, float Py );
	
	
}; // namespace

#endif
