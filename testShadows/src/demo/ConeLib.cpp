#include "stdafx.h"

// from home.comcast.net/~tom_forsyth/papers/cone_library.cpp

#include "ConeLib.h"



bool ViewCone::IntersectsSphere( const SVector3& pos, float r ) const
{
	// from www.geometrictools.com/Documentation/IntersectionSphereCone.pdf
	SVector3 u = -(r * this->invSinAngle) * this->axis;
	SVector3 d = pos - u;
	float dsqr = d.lengthSq();
	float e = this->axis.dot(d);
	if( e > 0 && e*e >= dsqr * this->cosAngle * this->cosAngle )
	{
		// pos is inside K''
		d = pos;
		dsqr = d.lengthSq();
		e = -this->axis.dot(d);
		if( e > 0 && e*e >= dsqr * this->sinAngle * this->sinAngle )
		{
			// pos is inside K'' and inside K'
			return dsqr <= r * r;
		}
		// pos inside K'' and outside K'
		return true;
	}
	// center is outside K''
	return false;
}



void ConeMake ( const SVector3 &v1, const SVector3 &v2, ViewCone& out )
{
	assert ( fabsf ( v1.lengthSq() - 1.0f ) < 0.01f );
	assert ( fabsf ( v2.lengthSq() - 1.0f ) < 0.01f );
	out.axis = SVector3( v1 + v2 ).getNormalized();
	out.cosAngle = v1.dot( out.axis );
	out.UpdateAngle();
}

void ConeMake ( const SVector3 &vCentre, float fRadius, ViewCone& out )
{
	float fOneOverLengthSq = 1.0f / vCentre.lengthSq();
	float fOneOverLength = sqrtf ( fOneOverLengthSq );
	out.axis = vCentre * fOneOverLength;

	if ( 1.0f <= ( fRadius * fOneOverLength ) )
	{
		// Point is inside the sphere.
		out.SetInvalidAngle();
		return;
	}

#if 0
	out.cosAngle = cosf ( asin ( fRadius / fLength ) );
#else
	// AdjacentLength = sqrt ( fLength * fLength - fRadius * fRadius )
	// CosAngle = AdjacentLength / fLength
	//  = sqrt ( fLength * fLength - fRadius * fRadius ) / fLength
	//  = sqrt ( 1.0f - ( fRadius * fRadius ) / ( fLength * fLength ) )
	out.cosAngle = sqrtf ( 1.0f - ( fRadius * fRadius ) * fOneOverLengthSq );
	assert ( fabsf ( out.cosAngle - cosf ( asinf ( fRadius * fOneOverLength ) ) ) < 0.00001f );
#endif
	out.UpdateAngle();
}



bool ConeIntersect ( const ViewCone &vc1, const ViewCone &vc2 )
{
	if ( ( vc1.cosAngle < 0.0f ) || ( vc2.cosAngle < 0.0f ) )
	{
		return true;
	}

	vc1.Check();
	vc2.Check();

	// Ugh - lots of acosfs. Can't see a better way to do it.
	float fThetaTotal = acosf ( vc1.axis.dot( vc2.axis ) );
	float fTheta1 = acosf ( vc1.cosAngle );
	float fTheta2 = acosf ( vc2.cosAngle );

	return ( fTheta1 + fTheta2 > fThetaTotal );
}



eConeUnionResult ConeUnion( ViewCone* out, const ViewCone &cone1, const ViewCone &cone2, bool bAlwaysSetResult )
{
	assert( out != NULL );

	const float fVeryCloseEnough = 0.00001f;
	const float fCloseEnough = 0.001f;

	// Just check if they share axis.
	float fOneDotTwo = cone2.axis.dot( cone1.axis );
	if ( fOneDotTwo > ( 1.0f - fVeryCloseEnough ) )
	{
		// Yep. OK, the test is really simple - which is bigger?
		if ( cone1.cosAngle < cone2.cosAngle )
		{
			if( bAlwaysSetResult )
			{
				out->axis = cone1.axis;
				out->cosAngle = cone1.cosAngle - fVeryCloseEnough;
				out->UpdateAngle();
			}
			return CUR_1ENCLOSES2;
		}
		else
		{
			if ( bAlwaysSetResult )
			{
				out->axis = cone2.axis;
				out->cosAngle = cone2.cosAngle - fVeryCloseEnough;
				out->UpdateAngle();
			}
			return CUR_2ENCLOSES1;
		}
	}
	else if ( fOneDotTwo < ( -1.0f + fVeryCloseEnough ) )
	{
		// They point in completely opposite directions.
		if ( bAlwaysSetResult )
		{
			out->SetInvalidAngle();
		}
		return CUR_NOBOUND;
	}


	// Find the plane that includes both axis - this is the plane that the final cone axis will lie on as well.
	SVector3 vPlaneNormal = cone2.axis.cross( cone1.axis ).getNormalized();

	// Now for each cone, find the "outer vector", which is the vector along the cone that lies in the plane,
	// furthest from the other cone's axis.
	// So define the vector vP = ( axis ^ vPlaneNormal ).norm()
	// So vP.axis = 0 and vP.vP=1.
	// Define:
	// vOuter = axis + lambda * ( ( axis ^ vPlaneNormal ).norm() )
	// and also:
	// ( vOuter * axis ) / vOuter.length() = cosAngle
	// thus:
	// lambda = +/- sqrt ( ( 1 - cosAngle^2 ) / ( cosAngle^2 ) )
	//
	// For cone1, use +ve lambda, for cone2, use -ve.

	SVector3 vP1 = vPlaneNormal.cross(cone1.axis ).getNormalized();
	float fCosAngleSquared1 = cone1.cosAngle * cone1.cosAngle;
	SVector3 vOuter1 = cone1.axis + vP1 * sqrtf ( ( 1.0f - fCosAngleSquared1 ) / fCosAngleSquared1 );
	vOuter1 = vOuter1.getNormalized();

	SVector3 vP2 = vPlaneNormal.cross( cone2.axis ).getNormalized();
	float fCosAngleSquared2 = cone2.cosAngle * cone2.cosAngle;
	SVector3 vOuter2 = cone2.axis - vP2 * sqrtf ( ( 1.0f - fCosAngleSquared2 ) / fCosAngleSquared2 );
	vOuter2 = vOuter2.getNormalized();

	// Check to see if either outer vector is actually inside the other cone.
	// If it is, then that cone completely encloses the other.
#ifdef _DEBUG
	float fDebug1 = vOuter2.dot( cone1.axis );
#endif
	if ( vOuter2.dot( cone1.axis ) + fVeryCloseEnough >= cone1.cosAngle )
	{
		// Obviously this means cone1 should be the wider one.
		assert ( cone1.cosAngle <= cone2.cosAngle + fCloseEnough );
		// And cone2's axis should be inside it as well.
		assert ( cone1.axis.dot( cone2.axis ) + fCloseEnough >= cone1.cosAngle );
		if ( bAlwaysSetResult )
		{
			*out = cone1;
		}
		return CUR_1ENCLOSES2;
	}
#ifdef _DEBUG
	float fDebug2 = vOuter2.dot( cone1.axis );
#endif
	if ( vOuter1.dot( cone2.axis ) + fVeryCloseEnough >= cone2.cosAngle )
	{
		// Obviously this means cone1 should be the wider one.
		assert ( cone2.cosAngle <= cone1.cosAngle + fCloseEnough );
		// And cone2's axis should be inside it as well.
		assert ( cone2.axis.dot( cone1.axis ) + fCloseEnough >= cone2.cosAngle );
		if ( bAlwaysSetResult )
		{
			*out = cone2;
		}
		return CUR_2ENCLOSES1;
	}

	// And if the cross-product of the two points the opposite direction to vPlaneNormal, then the bounding
	// cone has turned "inside out" and covers an angle more than 180 degrees.
	if ( vOuter1.cross( vOuter2 ).dot( vPlaneNormal ) >= 0.0f )
	{
		if ( bAlwaysSetResult )
		{
			out->SetInvalidAngle();
		}
		return CUR_NOBOUND;
	}

#ifdef _DEBUG
	// Taking copies coz presult might be one of the sources.
	float fCosAngle1 = cone1.cosAngle;
	float fCosAngle2 = cone2.cosAngle;
#endif

	// And now find the average of those two - that is the centre of the new cone.
	out->axis = SVector3( vOuter1 + vOuter2 ).getNormalized();
	// And the angle.
	out->cosAngle = out->axis.dot( vOuter1 );
	// Of course it shouldn't matter which you use to find the angle.
#ifdef _DEBUG
	float f = out->axis.dot( vOuter2 );
	// The smaller cosAngle, the bigger the allowable error.
	float fAllowedError = ( 1.0f - fabsf ( out->cosAngle ) ) * 0.2f + 0.001f;
	assert ( fabsf ( out->cosAngle - f ) < fAllowedError );
#endif

	// And obviously the resulting cone can't be narrower than either of the two sources.
	// remember that narrower = higher value)
	assert( out->cosAngle <= fCosAngle1 + fCloseEnough );
	assert( out->cosAngle <= fCosAngle2 + fCloseEnough );

	out->UpdateAngle();

	// All done.
	return CUR_NORMAL;
}



bool DoesCone2EncloseCone1( const ViewCone &cone1, const ViewCone &cone2 )
{
	const float fVeryCloseEnough = 0.00001f;
	const float fCloseEnough = 0.001f;

#ifdef _DEBUG
	ViewCone debugResCone;
	eConeUnionResult debugRes = ConeUnion( &debugResCone, cone1, cone2, false );
#endif

	// Just check if they share axis.
	float fOneDotTwo = cone2.axis.dot( cone1.axis );
	if ( fOneDotTwo > ( 1.0f - fVeryCloseEnough ) )
	{
		// Yep. OK, the test is really simple - which is bigger?
		if ( cone1.cosAngle < cone2.cosAngle )
		{
			//return CUR_1ENCLOSES2;
			assert ( debugRes == CUR_1ENCLOSES2 );
			return false;
		}
		else
		{
			//return CUR_2ENCLOSES1;
			assert ( debugRes == CUR_2ENCLOSES1 );
			return true;
		}
	}
	else if ( fOneDotTwo < ( -1.0f + fVeryCloseEnough ) )
	{
		// They point in completely opposite directions.
		//return CUR_NOBOUND;
		assert ( debugRes == CUR_NOBOUND );
		return false;
	}


	// Find the plane that includes both axis - this is the plane that the final cone axis will lie on as well.
	SVector3 vPlaneNormal = cone2.axis.cross( cone1.axis ).getNormalized();

	// Now for each cone, find the "outer vector", which is the vector along the cone that lies in the plane,
	// furthest from the other cone's axis.
	// So define the vector vP = ( axis ^ vPlaneNormal ).norm()
	// So vP.axis = 0 and vP.vP=1.
	// Define:
	// vOuter = axis + lambda * ( ( axis ^ vPlaneNormal ).norm() )
	// and also:
	// ( vOuter * axis ) / vOuter.length() = cosAngle
	// thus:
	// lambda = +/- sqrt ( ( 1 - cosAngle^2 ) / ( cosAngle^2 ) )
	//
	// For cone1, use +ve lambda, for cone2, use -ve.

	SVector3 vP1 = vPlaneNormal.cross( cone1.axis ).getNormalized();
	float fCosAngleSquared1 = cone1.cosAngle * cone1.cosAngle;
	SVector3 vOuter1 = cone1.axis + vP1 * sqrtf ( ( 1.0f - fCosAngleSquared1 ) / fCosAngleSquared1 );
	vOuter1 = vOuter1.getNormalized();

#ifdef _DEBUG
	SVector3 vP2 = vPlaneNormal.cross( cone2.axis ).getNormalized();
	float fCosAngleSquared2 = cone2.cosAngle * cone2.cosAngle;
	SVector3 vOuter2 = cone2.axis - vP2 * sqrtf ( ( 1.0f - fCosAngleSquared2 ) / fCosAngleSquared2 );
	vOuter2 = vOuter2.getNormalized();
#endif

	// Check to see if either outer vector is actually inside the other cone.
	// If it is, then that cone completely encloses the other.
#ifdef _DEBUG
	float fDebug2 = vOuter2.dot( cone1.axis );
#endif
	if ( vOuter1.dot( cone2.axis ) + fVeryCloseEnough >= cone2.cosAngle )
	{
		// Obviously this means cone1 should be the wider one.
		assert ( cone2.cosAngle <= cone1.cosAngle + fCloseEnough );
		// And cone2's axis should be inside it as well.
		assert ( cone2.axis.dot( cone1.axis ) + fCloseEnough >= cone2.cosAngle );
		//return CUR_2ENCLOSES1;
		assert ( debugRes == CUR_2ENCLOSES1 );
		return true;
	}

#if 0
	float fDebug1 = vOuter2.dot( cone1.axis );
	if ( vOuter2.dot( cone1.axis ) + fVeryCloseEnough >= cone1.cosAngle )
	{
		// Obviously this means cone1 should be the wider one.
		assert ( cone1.cosAngle <= cone2.cosAngle + fCloseEnough );
		// And cone2's axis should be inside it as well.
		assert ( cone1.axis.dot( cone2.axis ) + fCloseEnough >= cone1.cosAngle );
		//return CUR_1ENCLOSES2;
		return false;
	}

	// And if the cross-product of the two points the opposite direction to vPlaneNormal, then the bounding
	// cone has turned "inside out" and covers an angle more than 180 degrees.
	if ( vOuter1.cross( vOuter2 ).dot( vPlaneNormal ) >= 0.0f )
	{
		//return CUR_NOBOUND;
		return false;
	}
#endif

	//return CUR_NORMAL;

	assert ( debugRes != CUR_2ENCLOSES1 );

	return false;
}


float ConeSizeOfTexture ( const ViewCone &vc, float fPixelDensity )
{
	float fTanTheta = FindTanFromCos ( vc.cosAngle );
	return fPixelDensity * fTanTheta;
}
