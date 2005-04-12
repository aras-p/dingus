// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_RENDER_HELPER_H
#define __PARTICLE_RENDER_HELPER_H

#include "../../math/Vector3.h"
#include "../../math/Matrix4x4.h"
#include "../../utils/Singleton.h"

namespace dingus {


// --------------------------------------------------------------------------

class CParticleRenderHelper : public CSingleton<CParticleRenderHelper> {
public:
	enum { CORNERS_COUNT = 4 };
public:
	void begin( const SMatrix4x4& camRotMatrix, float particleSize );
		
	SVector3 const& getTransformedCorner( int corner ) const { return mTransformedCorners[corner]; }

private:
	IMPLEMENT_SIMPLE_SINGLETON(CParticleRenderHelper);

private:
	SVector3	mTransformedCorners[CORNERS_COUNT];
};


// --------------------------------------------------------------------------

class CRotParticleRenderHelper : public CSingleton<CRotParticleRenderHelper> {
public:
	enum { CORNERS_COUNT = 4 };
	enum { ROTATIONS_COUNT = 64 };
	
public:
	void begin( const SMatrix4x4& camRotMatrix );

	int getRotationIndex( float rot ) const { return (int)(rot*SPIN2INDEX) & (ROTATIONS_COUNT-1); }

	SVector3 const& getTransformedCorner( int rot, int corner ) const { return mTransformedCorners[rot][corner]; }

private:
	CRotParticleRenderHelper();
	IMPLEMENT_SIMPLE_SINGLETON(CRotParticleRenderHelper);
	
private:
	SVector3	mCorners[ROTATIONS_COUNT][CORNERS_COUNT];
	SVector3	mTransformedCorners[ROTATIONS_COUNT][CORNERS_COUNT];
	static const float	SPIN2INDEX;
};


// --------------------------------------------------------------------------

/*
class CLineParticleHelper {
public:
	virtual void begin( const SVector3& cameraPosition );

	SVector3 getPerpendicular( const SVector3& position, const SVector3& direction, float width );
	SVector3 getStripPerpendicular( const SVector3& position, const SVector3& direction, float width );

	const SVector3& getPreviousStripPerpendicular() const { return mPrevP; }

private:
	SVector3 mCameraPosition;
	SVector3 mPrevP;
	SVector3 mV;
};
*/


}; // namespace

#endif
