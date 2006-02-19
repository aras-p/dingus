#include "stdafx.h"
#include "Hull.h"

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

// Returns our "focused region of interest": frustum, clipped by scene bounds,
// extruded towards light and clipped by scene bounds again.
void CalculateFocusedLightHull( const SMatrix4x4& invViewProj, const SVector3& lightDir, const CAABox& sceneAABB )
{
	SVector3 viewFrustum[8];
	CalculateFrustumCoords( viewFrustum, invViewProj );
}

void CalculateFrustumHull( HullObject& obj, const SVector3 p[0] )
{
	obj.verts.insert( obj.verts.end(), &p[0], &p[8] );
	obj.faces.resize( 6 );
	for( int i = 0; i < 6; ++i )
		obj.faces[i].idx.resize( 4 );

	// near
	obj.faces[0].idx[0] = 0;
	obj.faces[0].idx[1] = 1;
	obj.faces[0].idx[2] = 3;
	obj.faces[0].idx[3] = 2;
	
	// far
	obj.faces[1].idx[0] = 4;
	obj.faces[1].idx[1] = 5;
	obj.faces[1].idx[2] = 7;
	obj.faces[1].idx[3] = 6;

	// sides
	obj.faces[2].idx[0] = 0;
	obj.faces[2].idx[1] = 1;
	obj.faces[2].idx[2] = 5;
	obj.faces[2].idx[3] = 4;
	
	obj.faces[3].idx[0] = 1;
	obj.faces[3].idx[1] = 3;
	obj.faces[3].idx[2] = 7;
	obj.faces[3].idx[3] = 5;

	obj.faces[4].idx[0] = 1;
	obj.faces[4].idx[1] = 3;
	obj.faces[4].idx[2] = 7;
	obj.faces[4].idx[3] = 5;

	//left poly ccw
	ps = obj->poly[2].points;
	copyVector3(ps[0],p[0]);
	copyVector3(ps[1],p[3]);
	copyVector3(ps[2],p[7]);
	copyVector3(ps[3],p[4]);
	//right poly ccw
	ps = obj->poly[3].points;
	copyVector3(ps[0],p[1]);
	copyVector3(ps[1],p[5]);
	copyVector3(ps[2],p[6]);
	copyVector3(ps[3],p[2]);
	//bottom poly ccw
	ps = obj->poly[4].points;
	copyVector3(ps[0],p[4]);
	copyVector3(ps[1],p[5]);
	copyVector3(ps[2],p[1]);
	copyVector3(ps[3],p[0]);
	//top poly ccw
	ps = obj->poly[5].points;
	copyVector3(ps[0],p[6]);
	copyVector3(ps[1],p[7]);
	copyVector3(ps[2],p[3]);
	copyVector3(ps[3],p[2]);
}
