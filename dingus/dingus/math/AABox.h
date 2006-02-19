// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATH_AABOX_H
#define __MATH_AABOX_H

#include "Vector3.h"
#include "Constants.h"
#include "Matrix4x4.h"

namespace dingus {


/**
 *  Axis aligned box.
 *
 *  Essentially two SVector3's - one for minimum, one for maximum.
 */
class CAABox {
public:
	/// Initializes to "none" AABB - minimum to big numbers, maximum to big negative numbers.
	CAABox()
	:	mMin(DINGUS_BIG_FLOAT,DINGUS_BIG_FLOAT,DINGUS_BIG_FLOAT),
		mMax(-DINGUS_BIG_FLOAT,-DINGUS_BIG_FLOAT,-DINGUS_BIG_FLOAT) { }

	/// Initializes to "none" AABB - minimum to big numbers, maximum to big negative numbers.
	void setNull() {
		mMin.x = mMin.y = mMin.z = DINGUS_BIG_FLOAT;
		mMax.x = mMax.y = mMax.z = -DINGUS_BIG_FLOAT;
	}

	/// Gets minimum vector.
	const SVector3& getMin() const { return mMin; }
	/// Gets minimum vector.
	SVector3& getMin() { return mMin; }
	/// Gets maximum vector.
	const SVector3& getMax() const { return mMax; }
	/// Gets maximum vector.
	SVector3& getMax() { return mMax; }

	/// Gets center vector.
	SVector3 getCenter() const { return (mMax+mMin)*0.5f; }

	/// Extends a box to include point
	void	extend( const SVector3& p ) {
		if( p.x < mMin.x ) mMin.x = p.x;
		if( p.y < mMin.y ) mMin.y = p.y;
		if( p.z < mMin.z ) mMin.z = p.z;
		if( p.x > mMax.x ) mMax.x = p.x;
		if( p.y > mMax.y ) mMax.y = p.y;
		if( p.z > mMax.z ) mMax.z = p.z;
	}
	/// Extends a box to include point and some more space
	void	extend( const SVector3& p, float more ) {
		if( p.x-more < mMin.x ) mMin.x = p.x-more;
		if( p.y-more < mMin.y ) mMin.y = p.y-more;
		if( p.z-more < mMin.z ) mMin.z = p.z-more;
		if( p.x+more > mMax.x ) mMax.x = p.x+more;
		if( p.y+more > mMax.y ) mMax.y = p.y+more;
		if( p.z+more > mMax.z ) mMax.z = p.z+more;
	}
	/// Extends a box to include other box
	void	extend( const CAABox& b ) {
		extend( b.getMin() );
		extend( b.getMax() );
	}
	
	/// Intersects box with another one
	void	intersect( const CAABox& b ) {
		if( b.mMin.x > mMin.x ) mMin.x = b.mMin.x;
		if( b.mMin.y > mMin.y ) mMin.y = b.mMin.y;
		if( b.mMin.z > mMin.z ) mMin.z = b.mMin.z;
		if( b.mMax.x < mMax.x ) mMax.x = b.mMax.x;
		if( b.mMax.y < mMax.y ) mMax.y = b.mMax.y;
		if( b.mMax.z < mMax.z ) mMax.z = b.mMax.z;
	}

	/// Transforms box with a matrix and calculates new box of the result
	void	transform( const SMatrix4x4& m ) {
		SVector3 vmin, vmax;
		D3DXVec3TransformCoord( &vmin, &mMin, &m );
		D3DXVec3TransformCoord( &vmax, &mMax, &m );
		D3DXVec3Minimize( &mMin, &vmin, &vmax );
		D3DXVec3Maximize( &mMax, &vmin, &vmax );
	}

	/// Tests if a point is inside this box
	bool isInside( const SVector3& p ) const {
		return
			p.x>=mMin.x && p.x<=mMax.x &&
			p.y>=mMin.y && p.y<=mMax.y &&
			p.z>=mMin.z && p.z<=mMax.z;
	}

	/**
	 *  Checks frustum cull.
	 *  @param world World matrix of AABB.
	 *  @param viewProj Combined view and projection matrices.
	 *  @return true if completely outside.
	 */
	bool frustumCull( const SMatrix4x4& world, const SMatrix4x4& viewProj ) const {
		enum {
			ClipLeft   = (1<<0),
			ClipRight  = (1<<1),
			ClipBottom = (1<<2),
			ClipTop    = (1<<3),
			ClipNear   = (1<<4),
			ClipFar    = (1<<5),
		};
		const int CORNERS = 8;
		SVector4 v[CORNERS];
		v[0].set( mMin.x, mMin.y, mMin.z, 1.0f );
		v[1].set( mMin.x, mMin.y, mMax.z, 1.0f );
		v[2].set( mMin.x, mMax.y, mMin.z, 1.0f );
		v[3].set( mMin.x, mMax.y, mMax.z, 1.0f );
		v[4].set( mMax.x, mMin.y, mMin.z, 1.0f );
		v[5].set( mMax.x, mMin.y, mMax.z, 1.0f );
		v[6].set( mMax.x, mMax.y, mMin.z, 1.0f );
		v[7].set( mMax.x, mMax.y, mMax.z, 1.0f );
		D3DXVec4TransformArray( v, sizeof(v[0]), v, sizeof(v[0]), &world, CORNERS );
		D3DXVec4TransformArray( v, sizeof(v[0]), v, sizeof(v[0]), &viewProj, CORNERS );
		
		int andFlags = 0xFFFF;
		int orFlags  = 0;
		for( int i = 0; i < CORNERS; ++i ) {
			int clip = 0;
			const SVector4& vv = v[i];
			if( vv.x < -vv.w )		clip |= ClipLeft;
			else if( vv.x > vv.w )	clip |= ClipRight;
			if( vv.y < -vv.w )		clip |= ClipBottom;
			else if( vv.y > vv.w )	clip |= ClipTop;
			if( vv.z < -vv.w )		clip |= ClipNear;
			else if( vv.z > vv.w )	clip |= ClipFar;
			andFlags &= clip;
			orFlags  |= clip;
		}
		if( orFlags == 0 )
			return false; // inside
		if( andFlags != 0 )
			return true; // outside
		return false; // partial
	}

	/**
	 *  Checks frustum cull.
	 *  @param worldViewProj World matrix of AABB combined with view and proj matrices.
	 *  @return true if completely outside.
	 */
	bool frustumCull( const SMatrix4x4& worldViewProj ) const {
		enum {
			ClipLeft   = (1<<0),
			ClipRight  = (1<<1),
			ClipBottom = (1<<2),
			ClipTop    = (1<<3),
			ClipNear   = (1<<4),
			ClipFar    = (1<<5),
		};
		const int CORNERS = 8;
		SVector4 v[CORNERS];
		v[0].set( mMin.x, mMin.y, mMin.z, 1.0f );
		v[1].set( mMin.x, mMin.y, mMax.z, 1.0f );
		v[2].set( mMin.x, mMax.y, mMin.z, 1.0f );
		v[3].set( mMin.x, mMax.y, mMax.z, 1.0f );
		v[4].set( mMax.x, mMin.y, mMin.z, 1.0f );
		v[5].set( mMax.x, mMin.y, mMax.z, 1.0f );
		v[6].set( mMax.x, mMax.y, mMin.z, 1.0f );
		v[7].set( mMax.x, mMax.y, mMax.z, 1.0f );
		D3DXVec4TransformArray( v, sizeof(v[0]), v, sizeof(v[0]), &worldViewProj, CORNERS );
		
		int andFlags = 0xFFFF;
		int orFlags  = 0;
		for( int i = 0; i < CORNERS; ++i ) {
			int clip = 0;
			const SVector4& vv = v[i];
			if( vv.x < -vv.w )		clip |= ClipLeft;
			else if( vv.x > vv.w )	clip |= ClipRight;
			if( vv.y < -vv.w )		clip |= ClipBottom;
			else if( vv.y > vv.w )	clip |= ClipTop;
			if( vv.z < -vv.w )		clip |= ClipNear;
			else if( vv.z > vv.w )	clip |= ClipFar;
			andFlags &= clip;
			orFlags  |= clip;
		}
		if( orFlags == 0 )
			return false; // inside
		if( andFlags != 0 )
			return true; // outside
		return false; // partial
	}

private:
	SVector3	mMin;
	SVector3	mMax;
};


}; // namespace


#endif
