#ifndef __AUDIO_CONTEXT_H
#define __AUDIO_CONTEXT_H

#include "../utils/Timer.h"

#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include "AudioListener.h"

namespace dingus {

class CSound;


class CAudioContext : public boost::noncopyable {
public:
	CAudioContext( HWND wnd );
	~CAudioContext();
	
	/// open the audio device
	bool open();
	/// close the audio device
	void close();

	bool isOpen() const { return mOpen; }

	///// reset all sounds
	//void reset();
	/// begin an audio frame
	bool beginScene( time_value time );
	/// update listener attributes
	void updateListener();
	/// start a sound
	void startSound( CSound& s );
	/// update a sound
	void updateSound( CSound& s );
	/// stop a sound
	void stopSound( CSound& s );
	/// end the audio frame
	void endScene();
	
	/// set the master volume (0.0 .. 1.0)
	void setMasterVolume( float v );
	/// get the master volume
	float getMasterVolume() const;
	
	/// Get dsound interface
	IDirectSound8& getDS() { return *mDS; }
	/// set primary buffer to a specified format
	bool setPrimaryBufferFmt( DWORD channels, DWORD freq, DWORD bitrate );

	const SAudioListener& getListener() const { return mListener; }
	SAudioListener& getListener() { return mListener; }

private:
	/// returns the 3D listener interface associated with primary buffer.
	bool get3DListener( IDirectSound3DListener** listener );
	
private:
	SAudioListener mListener;

	bool		mOpen;
	bool		mInBeginScene;
	float		mMasterVolume;
	bool		mMasterVolumeDirty;
	time_value	mMasterVolumeChangedTime;
	time_value	mCurrTime;

	// DSound specific
	IDirectSound8*	mDS;
	HWND mHWnd;
	IDirectSound3DListener8* m3DListener;
	DS3DLISTENER m3DProps;
	time_value mLastStreamUpdateCheck;
};

extern CAudioContext* G_AUDIOCTX;


// --------------------------------------------------------------------------

inline void CAudioContext::setMasterVolume( float v )
{
	mMasterVolume = v;
	mMasterVolumeDirty = true;
	mMasterVolumeChangedTime = mCurrTime;
}

inline float CAudioContext::getMasterVolume() const
{
	return mMasterVolume;
}


}; // namespace


#endif	  


