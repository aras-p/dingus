// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ODE_MARSHAL_H
#define __ODE_MARSHAL_H

//
// Data conversion between dingus and ODE
//

#include <ode/ode.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

namespace dingus {
	
typedef dBodyID			TPhysBodyID;
typedef dJointGroupID	TPhysJointGroupID;
typedef dJointID		TPhysJointID;
typedef dWorldID		TPhysWorldID;
typedef dGeomID			TCollidableID;
typedef dSpaceID		TCollidableContainerID;


namespace odemarshal {

struct SODEVector3 {
	dVector3	v;
};

// --------------------------------------------------------------------------

inline void matrixFromMatrix3( const dMatrix3 rot, SMatrix4x4& out )
{
	out._11 = float(rot[0]); out._21 = float(rot[1]); out._31 = float(rot[2]);
	out._12 = float(rot[4]); out._22 = float(rot[5]); out._32 = float(rot[6]);
	out._13 = float(rot[8]); out._23 = float(rot[9]); out._33 = float(rot[10]);
	out._41 = 0.0f; out._42 = 0.0f; out._43 = 0.0f;
	out._14 = 0.0f; out._24 = 0.0f; out._34 = 0.0f; out._44 = 1.0f;
}

inline void matrixToMatrix3( const SMatrix4x4& mat, dMatrix3 rot )
{
	rot[0]   = mat._11;
	rot[1]   = mat._21;
	rot[2]   = mat._31;
	rot[3]   = 0;
	rot[4]   = mat._12;
	rot[5]   = mat._22;
	rot[6]   = mat._32;
	rot[7]   = 0;
	rot[8]   = mat._13;
	rot[9]   = mat._23;
	rot[10]  = mat._33;
	rot[11]  = 0;
}

// --------------------------------------------------------------------------

inline void quatFromQuat( const dQuaternion src, SQuaternion& dst )
{
	// TBD: are you sure?
	dst.set( float(src[1]), float(src[2]), float(src[3]), float(src[0]) );
}

inline void quatFromQuat( const SQuaternion& src, dQuaternion dst )
{
	// TBD: are you sure?
	dst[0] = src.w;
	dst[1] = src.x;
	dst[2] = src.y;
	dst[3] = src.z;
}

// --------------------------------------------------------------------------

inline SVector3 vec3FromVector3( const dVector3 in )
{
	return SVector3( float(in[0]), float(in[1]), float(in[2]) );
}

inline void vec3FromVector3( const dVector3 in, SVector3& out )
{
	out.set( float(in[0]), float(in[1]), float(in[2]) );
}

inline void vec3ToVector3( const SVector3& in, dVector3 out )
{
	out[0] = in.x;
	out[1] = in.y;
	out[2] = in.z;
}

inline SODEVector3 vec3ToVector3( const SVector3& in )
{
	SODEVector3 v;
	v.v[0] = in.x;
	v.v[1] = in.y;
	v.v[2] = in.z;
	return v;
}

inline void vec4FromVector4( const dVector4 in, SVector4& out )
{
	out.set( float(in[0]), float(in[1]), float(in[2]), float(in[3]) );
}

inline void vec4ToVector4( const SVector4& in, dVector4 out )
{
	out[0] = in.x;
	out[1] = in.y;
	out[2] = in.z;
	out[3] = in.w;
}

// --------------------------------------------------------------------------

inline SVector3 vec3FromPtr( const dReal* in )
{
	return SVector3( float(in[0]), float(in[1]), float(in[2]) );
}

inline void vec3FromPtr( const dReal* in, SVector3& out )
{
	out.set( float(in[0]), float(in[1]), float(in[2]) );
}

inline SQuaternion quatFromPtr( const dReal* in )
{
	const dQuaternion& quat = *(const dQuaternion*)in;
	SQuaternion out;
	odemarshal::quatFromQuat( quat, out );
	return out;
}

inline void quatFromPtr( const dReal* in, SQuaternion& out )
{
	const dQuaternion& quat = *(const dQuaternion*)in;
	odemarshal::quatFromQuat( quat, out );
}

		
}; // namespace odemarshal

}; // namespace

#endif
