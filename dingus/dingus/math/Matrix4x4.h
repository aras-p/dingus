// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATRIX_44_H
#define __MATRIX_44_H

#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

namespace dingus {


// --------------------------------------------------------------------------

/**
 *  4x4 matrix.
 */
struct SMatrix4x4 : public D3DXMATRIX {
public:
	SMatrix4x4();
	SMatrix4x4( const float* f );
	SMatrix4x4( const D3DXFLOAT16* f );
	SMatrix4x4( const D3DMATRIX& m );
	SMatrix4x4( const D3DXMATRIX& m );
	SMatrix4x4( float _11, float _12, float _13, float _14,
				float _21, float _22, float _23, float _24,
				float _31, float _32, float _33, float _34,
				float _41, float _42, float _43, float _44 );
	SMatrix4x4( const SVector3& pos, const SQuaternion& rot );

    operator D3DXMATRIX&();
    operator const D3DXMATRIX&() const;

	const SVector3& getAxisX() const;
	SVector3& getAxisX();
	const SVector3& getAxisY() const;
	SVector3& getAxisY();
	const SVector3& getAxisZ() const;
	SVector3& getAxisZ();
	const SVector3& getOrigin() const;
	SVector3& getOrigin();

	void	identify();
	void	transpose( SMatrix4x4& dest ) const;
	void	transpose();
	void	rotationQuat( const SQuaternion& q );
	void	invert();

	/** Calculates Y and Z axis vectors from X. Uses planeSpace() from Vector3. */
	void	spaceFromAxisX();
	/** Calculates X and Z axis vectors from Y. Uses planeSpace() from Vector3. */
	void	spaceFromAxisY();
	/** Calculates X and Y axis vectors from Z. Uses planeSpace() from Vector3. */
	void	spaceFromAxisZ();

	// Avoid temporary: this = a * b
	void	fromMultiply( const SMatrix4x4& a, const SMatrix4x4& b );
};


inline SMatrix4x4::SMatrix4x4() : D3DXMATRIX() { };
inline SMatrix4x4::SMatrix4x4( const float *f ) : D3DXMATRIX(f) { };
inline SMatrix4x4::SMatrix4x4( const D3DXFLOAT16 *f ) : D3DXMATRIX(f) { };
inline SMatrix4x4::SMatrix4x4( const D3DXMATRIX& v ) : D3DXMATRIX(v) { };
inline SMatrix4x4::SMatrix4x4( float _11, float _12, float _13, float _14,
				float _21, float _22, float _23, float _24,
				float _31, float _32, float _33, float _34,
				float _41, float _42, float _43, float _44 )
				: D3DXMATRIX(_11,_12,_13,_14,_21,_22,_23,_24,_31,_32,_33,_34,_41,_42,_43,_44) { }
inline SMatrix4x4::SMatrix4x4( const SVector3& pos, const SQuaternion& rot )
{
	D3DXMatrixRotationQuaternion(this,&rot);
	getOrigin() = pos;
}

inline SMatrix4x4::operator D3DXMATRIX&() { return *this; }
inline SMatrix4x4::operator const D3DXMATRIX&() const { return *this; }

inline const SVector3& SMatrix4x4::getAxisX() const { return *(SVector3*)&_11; };
inline SVector3& SMatrix4x4::getAxisX() { return *(SVector3*)&_11; };
inline const SVector3& SMatrix4x4::getAxisY() const { return *(SVector3*)&_21; };
inline SVector3& SMatrix4x4::getAxisY() { return *(SVector3*)&_21; };
inline const SVector3& SMatrix4x4::getAxisZ() const { return *(SVector3*)&_31; };
inline SVector3& SMatrix4x4::getAxisZ() { return *(SVector3*)&_31; };
inline const SVector3& SMatrix4x4::getOrigin() const { return *(SVector3*)&_41; };
inline SVector3& SMatrix4x4::getOrigin() { return *(SVector3*)&_41; };

inline void SMatrix4x4::identify() { D3DXMatrixIdentity( this ); }
inline void SMatrix4x4::transpose( SMatrix4x4& dest ) const { D3DXMatrixTranspose(&dest,this); }
inline void SMatrix4x4::transpose() { D3DXMatrixTranspose(this,this); }
inline void SMatrix4x4::rotationQuat( const SQuaternion& q ) { D3DXMatrixRotationQuaternion(this,&q); }
inline void SMatrix4x4::invert() { D3DXMatrixInverse(this,NULL,this); }

inline void SMatrix4x4::spaceFromAxisX() { getAxisX().planeSpace( getAxisY(), getAxisZ() ); }
inline void SMatrix4x4::spaceFromAxisY() { getAxisY().planeSpace( getAxisZ(), getAxisX() ); }
inline void SMatrix4x4::spaceFromAxisZ() { getAxisZ().planeSpace( getAxisX(), getAxisY() ); }

inline void SMatrix4x4::fromMultiply( const SMatrix4x4& a, const SMatrix4x4& b ) { D3DXMatrixMultiply( this, &a, &b ); }

}; // namespace

#endif
