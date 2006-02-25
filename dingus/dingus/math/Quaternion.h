// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __QUATERNION_H
#define __QUATERNION_H


namespace dingus {



struct SQuaternion : public D3DXQUATERNION {
public:
	SQuaternion() : D3DXQUATERNION() { }
	SQuaternion( const float* f ) : D3DXQUATERNION(f) { }
	SQuaternion( float x, float y, float z, float w ) : D3DXQUATERNION(x,y,z,w) { }
	SQuaternion( const D3DXQUATERNION& q ) : D3DXQUATERNION(q.x,q.y,q.z,q.w) { };

    operator D3DXQUATERNION&() { return *this; }
    operator const D3DXQUATERNION&() const { return *this; }
	
	void	barycentric( const SQuaternion& q1, const SQuaternion& q2, const SQuaternion& q3, float f, float g ) { D3DXQuaternionBaryCentric(this,&q1,&q2,&q3,f,g); }
	void	conjugate() { D3DXQuaternionConjugate(this,this); }
	float	dot( const SQuaternion& q ) const { return D3DXQuaternionDot(this,&q); }
	void	identify() { D3DXQuaternionIdentity(this); }
	void	inverse() { D3DXQuaternionInverse(this,this); }
	bool	isIdentity() const { return D3DXQuaternionIsIdentity(this) ? true : false; }
	float	length() const { return D3DXQuaternionLength(this); }
	float	lengthSq() const { return D3DXQuaternionLengthSq(this); }
	void	normalize() { D3DXQuaternionNormalize(this,this); }
	
	void	slerp( const SQuaternion& q0, const SQuaternion& q1, float t )
	{
		D3DXQuaternionSlerp(this,&q0,&q1,t);
		/*
		float c = D3DXQuaternionDot( &q0, &q1 );

		if( c > 1.0f ) c = 1.0f;
		else if( c < -1.0f ) c = -1.0f;

		double ang = acos(c);
		if( abs(ang) < 1.0e-5 )
		{
			*this = q0;
			return;
		}

		double s = sin( ang );
		double is = 1.0f / s;

		*this =
			q0 * (sin((1 - t) * ang) * is) +
			q1 * (sin(t * ang) * is);
		*/
	}
	
	void	nlerp( const SQuaternion& a, const SQuaternion& b, float t )
	{
		// expand everything instead of using operators - seems
		// to be slightly faster
		const float oneMt = 1.0f - t;
		x = a.x * oneMt; y = a.y * oneMt; z = a.z * oneMt; w = a.w * oneMt;
		if( a.dot(b) >= 0.0f )
		{
			x += b.x * t; y += b.y * t; z += b.z * t; w += b.w * t;
		}
		else
		{
			x -= b.x * t; y -= b.y * t; z -= b.z * t; w -= b.w * t;
		}
		normalize();
	}

	void	set( float xx, float yy, float zz, float ww ) { x=xx; y=yy; z=zz; w=ww; }
};

}; // namespace


#endif
