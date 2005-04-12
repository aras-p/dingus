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
	void	slerp( const SQuaternion& a, const SQuaternion& b, float t ) { D3DXQuaternionSlerp(this,&a,&b,t); }

	void	set( float xx, float yy, float zz, float ww ) { x=xx; y=yy; z=zz; w=ww; }
};

}; // namespace


#endif
