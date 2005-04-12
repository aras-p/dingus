#ifndef __SOUND_H
#define __SOUND_H

#include "../math/Matrix4x4.h"
#include "../utils/MemoryPool.h"
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>


namespace dingus {

class CSoundResource;


/**
 *  Sound holds parameters for a sound instance. Usually a "game object" holds
 *  one or more CSound objects for all the sounds it has to play.
 */
class CSound : public boost::noncopyable {
public:
	CSound( CSoundResource& sr );

	/// start the sound
	void start();
	/// stop the sound
	void stop();
	/// update the sound
	void update();
	/// return true if sound is playing
	bool isPlaying();

	/// start if not playing, else update
	void startOrUpdate() {
		if( isPlaying() )
			update();
		else
			start();
	}

	/// set the mLooping behaviour
	void setLooping( bool b ) { mLooping = b; }
	/// get the mLooping behaviour
	bool isLooping() const { return mLooping; }

	/// set the sound mPriority for voice management
	void setPriority( int pri ) { mPriority = pri; }
	/// get the sound mPriority for voice management
	int getPriority() const { return mPriority; }

	/// set the playback mVolume
	void setVolume( float v );
	/// get the playback mVolume
	float getVolume() const { return mVolume; }

	/// set world mTransform
	void setTransform( const SMatrix4x4& m );
	/// get world mTransform
	const SMatrix4x4& getTransform() const { return mTransform; }

	/// set the mVelocity
	void setVelocity( const SVector3& v );
	/// get the mVelocity
	const SVector3& getVelocity() const { return mVelocity; }

	/// set the minimum distance (sound doesn't get louder when closer)
	void setMinDist( float d );
	/// get the mimimum distance
	float getMinDist() const { return mMinDist; }

	/// set the maximum distance (sound will not be audible beyond that distance)
	void setMaxDist(float d);
	/// get the maximum distance
	float getMaxDist() const { return mMaxDist; }

	/// set the inside cone angle in degrees
	void setInsideConeAngle(int a);
	/// get the inside cone angle
	int getInsideConeAngle() const { return mInsideConeAngle; }

	/// set the outside cone angle in degrees
	void setOutsideConeAngle(int a);
	/// get the outside cone angle
	int getOutsideConeAngle() const { return mOutsideConeAngle; }

	/// set the cone outside mVolume (0.0 .. 1.0)
	void setConeOutsideVolume(float v);
	/// get the cone outside mVolume
	float getConeOutsideVolume() const { return mConeOutsideVolume; }

	/// copy audio attributes from another sound object
	void copyFrom( const CSound& r );
	
private:
	/// Convert a linear volume between 0.0f and 1.0f into a Dezibel based dsound volume
	static LONG calcDSVolume( float vol );

	/// Get volume as dsound volume (update if dirty).
	LONG getDSVolume();
	/// Get dsound 3d buffer props (update if dirty).
	const DS3DBUFFER* CSound::getDS3DProps();

private:
	DECLARE_POOLED_ALLOC(dingus::CSound);

private:
	static int	mIDGenerator;
	
	// General sound props
	SMatrix4x4	mTransform;
	SVector3	mVelocity;
	CSoundResource*	mSoundResource;
	int		mPriority;
	float	mVolume;
	float	mMinDist;
	float	mMaxDist;
	int		mInsideConeAngle;
	int		mOutsideConeAngle;
	float	mConeOutsideVolume;
	bool	mLooping;

	int		mBufferIndex;
	int		mSoundID;
	
	bool mVolumeDirty;
	bool m3DPropsDirty;

	// DSound props
	DS3DBUFFER mDS3DProps;
	LONG	mDSVolume;
	DWORD	mDS3DMode;

};

// --------------------------------------------------------------------------


inline void CSound::copyFrom( const CSound& r )
{
	mTransform = r.mTransform;
	mVelocity = r.mVelocity;
	mSoundResource = r.mSoundResource;
	mPriority = r.mPriority;
	mVolume = r.mVolume;
	mMinDist = r.mMinDist;
	mMaxDist = r.mMaxDist;
	mInsideConeAngle = r.mInsideConeAngle;
	mOutsideConeAngle = r.mOutsideConeAngle;
	mConeOutsideVolume = r.mConeOutsideVolume;
	mLooping = r.mLooping;

	mVolumeDirty = true;
	m3DPropsDirty = true;

	mDS3DMode = r.mDS3DMode;
}


inline void CSound::setVolume( float v )
{
	mVolume = v;
	mVolumeDirty = true;
}

inline void CSound::setTransform( const SMatrix4x4& m )
{
	mTransform = m;
	m3DPropsDirty = true;
}

inline void CSound::setVelocity( const SVector3& v )
{
	mVelocity = v;
	m3DPropsDirty = true;
}

inline void CSound::setMinDist( float d )
{
	mMinDist = d;
	m3DPropsDirty = true;
}

inline void CSound::setMaxDist( float d )
{
	mMaxDist = d;
	m3DPropsDirty = true;
}

inline void CSound::setInsideConeAngle( int a )
{
	mInsideConeAngle = a;
	m3DPropsDirty = true;
}

inline void CSound::setOutsideConeAngle( int a )
{
	mOutsideConeAngle = a;
	m3DPropsDirty = true;
}

inline void CSound::setConeOutsideVolume( float v )
{
	mConeOutsideVolume = v;
	m3DPropsDirty = true;
}

inline LONG CSound::calcDSVolume( float vol )
{
	float scaledVolume = 0.0f;
	if( vol > 0.0f ) {
		const float minVol = 0.6f;
		const float maxVol = 1.0f;
		scaledVolume = minVol + (vol * (maxVol - minVol));
	}
	float dsVol = DSBVOLUME_MIN + ((DSBVOLUME_MAX - DSBVOLUME_MIN) * scaledVolume);
	return (LONG)dsVol;
}

inline LONG CSound::getDSVolume()
{
	if( mVolumeDirty ) {
		mVolumeDirty = false;
		mDSVolume = calcDSVolume( mVolume );
	}
	return mDSVolume;
}

}; // namespace

#endif
