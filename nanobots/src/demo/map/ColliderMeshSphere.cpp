#include "stdafx.h"
#include <dingus/math/Plane.h>

#include "Collider.h"

#include <limits>

using namespace dingus;



// Ripped from Opcode 1.1.
static bool GetContactData( const SVector3& Center, float r, const SVector3& Origin, const SVector3 Edge0, const SVector3& Edge1, float& Dist, float& u, float& v ) {
	SVector3 Diff = Origin - Center;
	
	float A00 = Edge0.dot(Edge0);
	float A01 = Edge0.dot(Edge1);
	float A11 = Edge1.dot(Edge1);
	
	float B0 = Diff.dot( Edge0 );
	float B1 = Diff.dot( Edge1 );
	
	float C = Diff.dot(Diff);
	
	float Det = fabsf(A00 * A11 - A01 * A01);
	u = A01 * B1 - A11 * B0;
	v = A01 * B0 - A00 * B1;
	
	float DistSq;
	
	if (u + v <= Det){
		if(u < 0.0f){
			if(v < 0.0f){	// region 4
				if(B0 < 0.0f){
					v = 0.0f;
					if (-B0 >= A00){
						u = 1.0f;
						DistSq = A00 + 2.0f * B0 + C;
					}
					else{
						u = -B0 / A00;
						DistSq = B0 * u + C;
					}
				}
				else{
					u = 0.0f;
					if(B1 >= 0.0f){
						v = 0.0f;
						DistSq = C;
					}
					else if(-B1 >= A11){
						v = 1.0f;
						DistSq = A11 + 2.0f * B1 + C;
					}
					else{
						v = -B1 / A11;
						DistSq = B1 * v + C;
					}
				}
			}
			else{  // region 3
				u = 0.0f;
				if(B1 >= 0.0f){
					v = 0.0f;
					DistSq = C;
				}
				else if(-B1 >= A11){
					v = 1.0f;
					DistSq = A11 + 2.0f * B1 + C;
				}
				else{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < 0.0f){  // region 5
			v = 0.0f;
			if (B0 >= 0.0f){
				u = 0.0f;
				DistSq = C;
			}
			else if (-B0 >= A00){
				u = 1.0f;
				DistSq = A00 + 2.0f * B0 + C;
			}
			else{
				u = -B0 / A00;
				DistSq = B0 * u + C;
			}
		}
		else{  // region 0
			// minimum at interior point
			if (Det == 0.0f){
				u = 0.0f;
				v = 0.0f;
				DistSq = DINGUS_MAX_FLOAT;
			}
			else{
				float InvDet = (1.0) / Det;
				u *= InvDet;
				v *= InvDet;
				DistSq = u * (A00 * u + A01 * v + (2.0) * B0) + v * (A01 * u + A11 * v + (2.0) * B1) + C;
			}
		}
	}
	else{
		float Tmp0, Tmp1, Numer, Denom;
		
		if(u < (0.0)){	// region 2
			Tmp0 = A01 + B0;
			Tmp1 = A11 + B1;
			if (Tmp1 > Tmp0){
				Numer = Tmp1 - Tmp0;
				Denom = A00 - (2.0) * A01 + A11;
				if (Numer >= Denom){
					u = (1.0);
					v = (0.0);
					DistSq = A00 + (2.0) * B0 + C;
				}
				else{
					u = Numer / Denom;
					v = (1.0) - u;
					DistSq = u * (A00 * u + A01 * v + (2.0) * B0) + v * (A01 * u + A11 * v + (2.0) * B1) + C;
				}
			}
			else{
				u = (0.0);
				if(Tmp1 <= (0.0)){
					v = (1.0);
					DistSq = A11 + (2.0) * B1 + C;
				}
				else if(B1 >= (0.0)){
					v = (0.0);
					DistSq = C;
				}
				else{
					v = -B1 / A11;
					DistSq = B1 * v + C;
				}
			}
		}
		else if(v < (0.0)){  // region 6
			Tmp0 = A01 + B1;
			Tmp1 = A00 + B0;
			if (Tmp1 > Tmp0){
				Numer = Tmp1 - Tmp0;
				Denom = A00 - (2.0) * A01 + A11;
				if (Numer >= Denom){
					v = (1.0);
					u = (0.0);
					DistSq = A11 + (2.0) * B1 + C;
				}
				else{
					v = Numer / Denom;
					u = (1.0) - v;
					DistSq =  u * (A00 * u + A01 * v + (2.0) * B0) + v * (A01 * u + A11 * v + (2.0) * B1) + C;
				}
			}
			else{
				v = (0.0);
				if (Tmp1 <= (0.0)){
					u = (1.0);
					DistSq = A00 + (2.0) * B0 + C;
				}
				else if(B0 >= (0.0)){
					u = (0.0);
					DistSq = C;
				}
				else{
					u = -B0 / A00;
					DistSq = B0 * u + C;
				}
			}
		}
		else{  // region 1
			Numer = A11 + B1 - A01 - B0;
			if (Numer <= (0.0)){
				u = (0.0);
				v = (1.0);
				DistSq = A11 + (2.0) * B1 + C;
			}
			else{
				Denom = A00 - (2.0) * A01 + A11;
				if (Numer >= Denom){
					u = (1.0);
					v = (0.0);
					DistSq = A00 + (2.0) * B0 + C;
				}
				else{
					u = Numer / Denom;
					v = (1.0) - u;
					DistSq = u * (A00 * u + A01 * v + (2.0) * B0) + v * (A01 * u + A11 * v + (2.0) * B1) + C;
				}
			}
		}
	}
	
	Dist = sqrtf(fabsf(DistSq));
	
	if (Dist <= r){
		Dist = r - Dist;
		return true;
	}
	else return false;
}


bool CMeshSphereCollider2::perform( CCollisionMesh& mesh, CCollisionSphere& sphere, CCollisionResult& result )
{
	Opcode::SphereCache sphereCache;
	bool s = mSphereMeshCollider.Collide( sphereCache, sphere.getSphere(), mesh.getOpcodeModel(), NULL, NULL );
	assert( s );

	if( mSphereMeshCollider.GetContactStatus() ) {
		// Number of colliding pairs and list of pairs
		int touchedPrimitiveCount = mSphereMeshCollider.GetNbTouchedPrimitives();
		const udword* touchedPrimitives = mSphereMeshCollider.GetTouchedPrimitives();

		result.mDirection.set(0,0,0);
		result.mDistance = 0;
		result.mPosition.set(0,0,0);

		float r = sphere.getRadius();
		SVector3 spherePos = sphere.getPosition();

		int reallyTouchedTriCount = 0;
		for( int i = 0; i < touchedPrimitiveCount; i++ ) {
			// fetch and transform a triangle
			const IndexedTriangle& idxtri = mesh.getOpcodeTriangles()[touchedPrimitives[i]];
			int i0 = idxtri.mVRef[0];
			int i1 = idxtri.mVRef[1];
			int i2 = idxtri.mVRef[2];
			SVector3 v0 = mesh.getVertex( i0 );
			SVector3 v1 = mesh.getVertex( i1 );
			SVector3 v2 = mesh.getVertex( i2 );

			// edges
			SVector3 vu = v1-v0;
			SVector3 vv = v2-v0;
			
			float depth;
			float u, v;
			if( !GetContactData(spherePos, r, v0, vu, vv, depth, u, v) ) {
				continue;	// Sphere doesnt hit triangle
			}
			float w = 1.0f - u - v;
			
			if( depth <= 0.0f ) {
				depth = 0.0f;
				continue;
			}

			SVector3 cpos = v0*w + v1*u + v2*v;
			result.mPosition += cpos;
			result.mDirection += vv.cross(vu).getNormalized() * depth;

			++reallyTouchedTriCount;
		}

		if( reallyTouchedTriCount == 0 )
			return false;

		result.mPosition /= reallyTouchedTriCount;

		// divide in square space
		result.mDistance = sqrtf( result.mDirection.lengthSq() / reallyTouchedTriCount );
		result.mDirection.normalize();

		return true;
	} else
		return false;
}

void CMeshSphereCollider2::perform( CCollisionMesh& mesh, CCollisionSphere& sphere, std::vector<int>& tris )
{
	Opcode::SphereCache sphereCache;
	bool s = mSphereMeshCollider.Collide( sphereCache, sphere.getSphere(), mesh.getOpcodeModel(), NULL, NULL );
	assert( s );
	tris.clear();
	if( mSphereMeshCollider.GetContactStatus() ) {
		int touchedPrimitiveCount = mSphereMeshCollider.GetNbTouchedPrimitives();
		const udword* touchedPrimitives = mSphereMeshCollider.GetTouchedPrimitives();

		tris.resize( touchedPrimitiveCount );
		for( int i = 0; i < touchedPrimitiveCount; i++ )
			tris[i] = touchedPrimitives[i];
	}
}
