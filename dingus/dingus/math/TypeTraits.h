// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef MATH_TYPE_TRAITS_H_
#define MATH_TYPE_TRAITS_H_

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "Interpolation.h"

namespace dingus {

/*
	Math type traits. Each traits class, math_type_traits<T>, has methods:
	static T identity(); - returns identity.
	static void identify( T& ); - makes given value identity.
	static R interpolate( T& a1, T& a2, TT& t ); - lerps.
	static R cm_interpolate( T& a0, T& a1, T& a2, T& a3, TT& t ); - catmull-rom interpolates.
*/

template<typename _T>
struct math_type_traits_base {
	template<typename _R, typename _TT>
	static inline _R interpolate( _T const& a1, _T const& a2, _TT const& t ) {
		return math_lerp<_R>( a1, a2, t );
	}
	template<typename _R, typename _TT>
	static inline _R cm_interpolate( _T const& a0, _T const& a1, _T const& a2, _T const& a3, _TT const& t ) {
		return math_catmull_rom<_R>( a0, a1, a2, a3, t );
	}
};


// --------------------------------------------------------------------------
//  basic traits

template<typename _T>
struct math_type_traits : public math_type_traits_base<_T> {
};


// --------------------------------------------------------------------------
//  float traits

template<>
struct math_type_traits<float> : public math_type_traits_base<float> {
	static inline float identity() { return 0.0f; }
	static inline void identify( float& val ) { val = identity(); }

	template<typename _TT>
	static inline float interpolate( float const& a1, float const& a2, _TT const& t ) {
		return math_lerp<float>( a1, a2, t );
	};
	template<typename _TT>
	static inline float cm_interpolate( float const& a0, float const& a1, float const& a2, float const& a3, _TT const& t ) {
		return math_catmull_rom<float>( a0, a1, a2, a3, t );
	};
};


// --------------------------------------------------------------------------
//  SVector3 traits

template<>
struct math_type_traits<SVector3> : public math_type_traits_base<SVector3> {
	static inline SVector3 identity() { return SVector3(0,0,0); }
	static inline void identify( SVector3& val ) { val = identity(); }

	template<typename _TT>
	static inline SVector3 interpolate( SVector3 const& a1, SVector3 const& a2, _TT const& t ) {
		return math_lerp<SVector3>( a1, a2, t );
	};
	template<typename _TT>
	static inline SVector3 cm_interpolate( SVector3 const& a0, SVector3 const& a1, SVector3 const& a2, SVector3 const& a3, _TT const& t ) {
		return math_catmull_rom<SVector3>( a0, a1, a2, a3, t );
	};
};


// --------------------------------------------------------------------------
//  SVector4 traits

template<>
struct math_type_traits<SVector4> : public math_type_traits_base<SVector4> {
	static inline SVector4 identity() { return SVector4(0,0,0,0); }
	static inline void identify( SVector4& val ) { val = identity(); }

	template<typename _TT>
	static inline SVector4 interpolate( SVector4 const& a1, SVector4 const& a2, _TT const& t ) {
		return math_lerp<SVector4>( a1, a2, t );
	};
	template<typename _TT>
	static inline SVector4 cm_interpolate( SVector4 const& a0, SVector4 const& a1, SVector4 const& a2, SVector4 const& a3, _TT const& t ) {
		return math_catmull_rom<SVector4>( a0, a1, a2, a3, t );
	};
};


// --------------------------------------------------------------------------
//  SQuaternion traits

template<>
struct math_type_traits<SQuaternion> : public math_type_traits_base<SQuaternion>  {
	static inline SQuaternion identity() { return SQuaternion(0,0,0,1); }
	static inline void identify( SQuaternion& val ) { val = identity(); }

	template<typename _TT>
	static inline SQuaternion interpolate( SQuaternion const& a1, SQuaternion const& a2, _TT const& t ) {
		SQuaternion qo;
		//qo.slerp( a1, a2, t ); // do not use slerp!
		qo.nlerp( a1, a2, t );
		return qo;
	};

	template<typename _TT>
	static inline SQuaternion cm_interpolate( SQuaternion const& a0, SQuaternion const& a1, SQuaternion const& a2, SQuaternion const& a3, _TT const& t ) {
		SQuaternion qa, qb, qc, qo;
		D3DXQuaternionSquadSetup( &qa, &qb, &qc, &a0, &a1, &a2, &a3 );
		return *D3DXQuaternionSquad( &qo, &a1, &qa, &qb, &qc, t );
	};
};


}; // namespace

#endif
