// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "Vector3.h"
#include "Matrix4x4.h"

namespace dingus {


class CTransform {
public:
	CTransform();
	CTransform( const SVector3& pos, const D3DXQUATERNION& rot );
	CTransform( CTransform const& r );
	CTransform const& operator=( CTransform const& rh );

	void identify();

	void toMatrix( SMatrix4x4& m ) const;

	/** This func shouldn't be used. Conversion from matrix to vec/quat is expensive. */
	void setFromMatrix( SMatrix4x4 const& m );

	void setPosition( SVector3 const& v ) { mPosition = v; }
	void setRotation( D3DXQUATERNION const& q ) { mRotation = q; }

	SVector3 const& getPosition() const { return mPosition; }
	SVector3& getPosition() { return mPosition; }
	D3DXQUATERNION const& getRotation() const { return mRotation; }
	D3DXQUATERNION& getRotation() { return mRotation; }

private:
	SVector3			mPosition;
	D3DXQUATERNION		mRotation;
};


}; // namespace 

#endif

