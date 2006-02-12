#ifndef __CONE_LIB_H
#define __CONE_LIB_H

// from home.comcast.net/~tom_forsyth/papers/cone_library.cpp


static inline float FindTanFromCos( float val )
{
	float tanTheta = sqrtf(1 - val*val) / val;
	assert( fabsf( tanTheta - tanf(acosf(val)) ) < 0.001f );
	return tanTheta;
}

static inline float FindSinFromCos( float val )
{
	// we're only interested in positive sine values for the cones anyway
	float sinTheta = sqrtf(1 - val*val);
	assert( fabsf( sinTheta - sinf(acosf(val)) ) < 0.001f );
	return sinTheta;
}



// A view cone originates at the origin, has its axis along axis (always
// normalized). All points on it are defined by
// (axis * P)/P.length() = cosAngle.
struct ViewCone
{
	SVector3 axis;
	float	cosAngle;
	float	sinAngle;
	float	invSinAngle;

	void	UpdateAngle()
	{
		sinAngle = FindSinFromCos( cosAngle );
		invSinAngle = 1.0f / sinAngle;
	}

	void Check() const { assert ( fabsf ( axis.lengthSq() - 1.0f ) < 0.0001f ); }
	
	void SetInvalidAngle()
	{
		cosAngle = -1.0f;
		sinAngle = 0.0f;
		invSinAngle = 1.0e20f;
	}
	bool IsInvalidAngle() const
	{
		return cosAngle <= -1.0f;
	}
	
	bool IntersectsSphere( const SVector3& pos, float r ) const;
};


enum eConeUnionResult
{
	CUR_NORMAL,			// A new bounding cone was found.
	CUR_1ENCLOSES2,		// ViewCone 1 encloses cone 2 already.
	CUR_2ENCLOSES1,		// ViewCone 2 encloses cone 1 already.
	CUR_NOBOUND,		// There is no way to union these - the cone would subtend more than 180 degrees.
};


// Make a cone from the union of two unit-length vectors
void ConeMake( const SVector3 &v1, const SVector3 &v2, ViewCone& out );
// Make a cone from a bounding sphere.
void ConeMake ( const SVector3 &vCentre, float fRadius, ViewCone& out );

//bool ConeIntersect ( const ViewCone &vc1, const ViewCone &vc2 );

// Returns the cone that bounds both the input cones.
eConeUnionResult ConeUnion( ViewCone* out, const ViewCone &cone1, const ViewCone &cone2, bool bAlwaysSetResult = false );
// This is a version of ConeUnion, mangled so that all it does is decide if one cone encloses the other.
bool DoesCone2EncloseCone1 ( const ViewCone &cone1, const ViewCone &cone2 );

// Calculate the size of a texture required for a given view cone and its pixel density.
float ConeSizeOfTexture ( const ViewCone &vc, float fPixelDensity );


#endif
