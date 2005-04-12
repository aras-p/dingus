// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VECTOR_2_H
#define __VECTOR_2_H

namespace dingus {

// --------------------------------------------------------------------------

/**
 *  2D vector.
 */
struct SVector2 : public D3DXVECTOR2 {
public:
	SVector2();
	SVector2( const float* f );
	SVector2( const D3DXFLOAT16* f );
	SVector2( float x, float y );
	SVector2( const D3DXVECTOR2& v );

    operator D3DXVECTOR2&();
    operator const D3DXVECTOR2&() const;

	void		set( float vx, float vy );
	float		length() const;
	float		lengthSq() const;
	float		dot( const SVector2& v ) const;
	void		normalize();
	SVector2	getNormalized() const;
};


inline SVector2::SVector2() : D3DXVECTOR2() { };
inline SVector2::SVector2( const float *f ) : D3DXVECTOR2(f) { };
inline SVector2::SVector2( const D3DXFLOAT16 *f ) : D3DXVECTOR2(f) { };
inline SVector2::SVector2( float vx, float vy ) : D3DXVECTOR2(vx,vy) { };
inline SVector2::SVector2( const D3DXVECTOR2& v ) : D3DXVECTOR2(v) { };

inline void SVector2::set( float vx, float vy ) { x=vx; y=vy; };
inline float SVector2::length() const { return D3DXVec2Length(this); };
inline float SVector2::lengthSq() const { return D3DXVec2LengthSq(this); };
inline float SVector2::dot( const SVector2& v ) const { return D3DXVec2Dot(this,&v); }
inline void	SVector2::normalize() {
	D3DXVec2Normalize( this, this );
}
inline SVector2 SVector2::getNormalized() const {
	SVector2 v;
	D3DXVec2Normalize( &v, this );
	return v;
}

inline SVector2::operator D3DXVECTOR2&() { return *this; }
inline SVector2::operator const D3DXVECTOR2&() const { return *this; }

}; // namespace

#endif
