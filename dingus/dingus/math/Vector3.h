// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VECTOR_3_H
#define __VECTOR_3_H

namespace dingus {

// --------------------------------------------------------------------------

/**
 *  3D vector.
 */
struct SVector3 : public D3DXVECTOR3 {
public:
	SVector3();
	SVector3( const float* f );
	SVector3( const D3DXFLOAT16* f );
	SVector3( float x, float y, float z );
	SVector3( const D3DXVECTOR3& v );

    operator D3DXVECTOR3&();
    operator const D3DXVECTOR3&() const;

	void		set( float vx, float vy, float vz );
	float		length() const;
	float		lengthSq() const;
	float		dot( const SVector3& v ) const;
	void		normalize();

	SVector3	cross( const SVector3& v ) const;
	SVector3	getNormalized() const;

	/**
	 *  Calculates plane space vectors from normal vector.
	 *
	 *  Takes this vector as "normal", generates P and Q vectors that are an
	 *  orthonormal basis for the plane space perpendicular to normal (so that
	 *  this, P and Q are all perpendicular to each other). Q will equal
	 *  cross(this, p). If this is not unit length then P will be unit length
	 *  but Q wont be.
	 */
	void		planeSpace( SVector3& p, SVector3& q ) const;
};


inline SVector3::SVector3() : D3DXVECTOR3() { };
inline SVector3::SVector3( const float *f ) : D3DXVECTOR3(f) { };
inline SVector3::SVector3( const D3DXFLOAT16 *f ) : D3DXVECTOR3(f) { };
inline SVector3::SVector3( float vx, float vy, float vz ) : D3DXVECTOR3(vx,vy,vz) { };
inline SVector3::SVector3( const D3DXVECTOR3& v ) : D3DXVECTOR3(v) { };

inline void SVector3::set( float vx, float vy, float vz ) { x=vx; y=vy; z=vz; };
inline float SVector3::length() const { return D3DXVec3Length(this); };
inline float SVector3::lengthSq() const { return D3DXVec3LengthSq(this); };
inline float SVector3::dot( const SVector3& v ) const { return D3DXVec3Dot(this,&v); }
inline SVector3 SVector3::cross( const SVector3& v ) const {
	SVector3 res;
	res.x = y * v.z - z * v.y;
    res.y = z * v.x - x * v.z;
    res.z = x * v.y - y * v.x;
	return res;
}
inline void	SVector3::normalize() {
	D3DXVec3Normalize( this, this );
}
inline SVector3 SVector3::getNormalized() const {
	SVector3 v;
	D3DXVec3Normalize( &v, this );
	return v;
}

inline SVector3::operator D3DXVECTOR3&() { return *this; }
inline SVector3::operator const D3DXVECTOR3&() const { return *this; }


}; // namespace

#endif
