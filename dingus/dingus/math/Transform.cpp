// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Transform.h"

using namespace dingus;


CTransform::CTransform()
{
	identify();
}

CTransform::CTransform( SVector3 const& pos, D3DXQUATERNION const& rot )
:	mPosition( pos ),
	mRotation( rot )
{
}

CTransform::CTransform( CTransform const& r )
:	mPosition( r.mPosition ),
	mRotation( r.mRotation )
{
}

CTransform const& CTransform::operator=( CTransform const& rh )
{
	mPosition = rh.mPosition;
	mRotation = rh.mRotation;
	return *this;
}

void CTransform::setFromMatrix( SMatrix4x4 const& m )
{
	mPosition = m.getOrigin();
	D3DXQuaternionRotationMatrix( &mRotation, &m );
}

void CTransform::identify()
{
	D3DXQuaternionIdentity( &mRotation );
	mPosition.set( 0,0,0 );
}

void CTransform::toMatrix( SMatrix4x4& m ) const
{
	D3DXMatrixRotationQuaternion( &m, &mRotation );
	m.getOrigin() = mPosition;
}


/**

Have a parent transform: Pv/Pq and child: Cv/Cq.

If we had matrices Pm and Cm, then child matrix in world space would be:
	CWm = Cm * PWm.

For transforms:
	CWm = (Cq, Cv) * PWm
	    = (Cq, Cv) * (Pq, Pw)
*/
