#include "stdafx.h"
#include "Sound.h"
#include "SoundResource.h"
#include "AudioContext.h"

using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CSound,128,false);


int CSound::mIDGenerator = 0;


CSound::CSound( CSoundResource& sr )
:	mVelocity(0,0,0),
	mSoundResource(&sr),
	mPriority(0),
	mVolume(1.0f),
	mMinDist(10.0f),
	mMaxDist(100.0f),
	mInsideConeAngle(0),
	mOutsideConeAngle(360),
	mConeOutsideVolume(1.0f),
	mLooping( false ),
	mBufferIndex( -1 ), mSoundID( ++mIDGenerator ),
	mVolumeDirty( true ), m3DPropsDirty( true ),
	mDSVolume(0), mDS3DMode(DS3DMODE_NORMAL)
{
	assert( &sr );
	mTransform.identify();

    memset( &mDS3DProps, 0, sizeof(mDS3DProps) );
    mDS3DProps.dwSize = sizeof(mDS3DProps);
}


void CSound::start()
{
	if( !G_AUDIOCTX->isOpen() )
		return;

	LONG dsvol = getDSVolume();
	const DS3DBUFFER* ds3D = getDS3DProps();
	
	// a streaming sound?
	if( mSoundResource->isStreaming() )
		mSoundResource->reset( 0, mLooping ); // TBD: nebula resets one-shot sounds also. Why?
	
	// playback flags
	int flags = 0;
	if( mLooping || mSoundResource->isStreaming() ) {
		flags |= DSBPLAY_LOOPING;
	}
	if( mSoundResource->isAmbient() ) {
		// play as 2D sound
		mBufferIndex = mSoundResource->play( mSoundID, mPriority, flags, dsvol, 0 );
		if( mBufferIndex < 0 ) {
			// TBD
			// n_printf("nDSoundServer3: failed to start 2D sound '%s'\n", GetFilename());
		}
	} else {
		// play as 3D sound
		//flags |= DSBPLAY_TERMINATEBY_PRIORITY;
		mBufferIndex = mSoundResource->play3D( mSoundID, *ds3D, mPriority, flags, dsvol, 0 );
		if( mBufferIndex < 0 ) {
			// TBD
			// n_printf("nDSoundServer3: failed to start 3D sound '%s'\n", GetFilename());
		}
	}
}

void CSound::stop()
{
	if( !G_AUDIOCTX->isOpen() )
		return;

	if( isPlaying() )
		mSoundResource->stop( mBufferIndex, mSoundID );
}

void CSound::update()
{
	if( !G_AUDIOCTX->isOpen() )
		return;

	LONG dsvol = getDSVolume();
	mSoundResource->setVolume( mBufferIndex, mSoundID, dsvol );
}

bool CSound::isPlaying()
{
	if( !G_AUDIOCTX->isOpen() )
		return false;
	return mSoundResource->isPlaying( mBufferIndex, mSoundID );
}

const DS3DBUFFER* CSound::getDS3DProps()
{
	if( m3DPropsDirty ) {
		m3DPropsDirty = false;
		mDS3DProps.vPosition.x = mTransform.getOrigin().x;
		mDS3DProps.vPosition.y = mTransform.getOrigin().y;
		mDS3DProps.vPosition.z = mTransform.getOrigin().z;
		mDS3DProps.vVelocity.x = mVelocity.x;
		mDS3DProps.vVelocity.y = mVelocity.y;
		mDS3DProps.vVelocity.z = mVelocity.z;
		mDS3DProps.dwInsideConeAngle = (DWORD)mInsideConeAngle;
		mDS3DProps.dwOutsideConeAngle = (DWORD)mOutsideConeAngle;
		mDS3DProps.vConeOrientation.x = mTransform.getAxisZ().x;
		mDS3DProps.vConeOrientation.y = mTransform.getAxisZ().y;
		mDS3DProps.vConeOrientation.z = mTransform.getAxisZ().z;
		mDS3DProps.lConeOutsideVolume = calcDSVolume( mConeOutsideVolume );
		mDS3DProps.flMinDistance = mMinDist;
		mDS3DProps.flMaxDistance = mMaxDist;
		mDS3DProps.dwMode = mDS3DMode;
	}
	return &mDS3DProps;
}
