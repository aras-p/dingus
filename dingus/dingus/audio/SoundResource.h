#ifndef __SOUND_RESOURCE_H
#define __SOUND_RESOURCE_H

#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include "../utils/MemoryPool.h"

namespace dingus {

class CSoundWaveFmt;


/**
*  CSoundResource is a container for sound data which can be played by the
*  audio system. The sound may be static or mStreaming, oneshot or mLooping.
*  A sound resource should be able play itself several times simultaneously,
*  the intended number of parallel "tracks" can be set by the user before
*  opening the resource.
*
*  Sound resources are shared and are referenced by CSound objects (there
*  should be one CSound per "sound instance", but several sound objects
*  should reference the same CSoundResource object).
*/
class CSoundResource : public boost::noncopyable {
public:
	CSoundResource();
	~CSoundResource();
	
	bool createResource( const std::string& fileName, int tracks, bool ambient, bool streaming );
	void deleteResource();

	/// stop all buffers
	//void stop();

	/**
	 *  @return Buffer index, or -1 on error.
	 */
	int play( int sndID, DWORD priority, DWORD flags, LONG volume, LONG freq = -1, LONG pan = 0 );

	/**
	 *  @return Buffer index, or -1 on error.
	 */
	int play3D( int sndID, const DS3DBUFFER& props3D, DWORD priority, DWORD flags, LONG volume, LONG freq = 0 );

	/**
	 *  Stops sound at given buffer index if it matches the sound ID.
	 */
	void	stop( int bufferIdx, int sndID );
	
	void	setVolume( int bufferIndex, int sndID, LONG volume );

	/// Resets sound at given buffer index.
	bool	reset( int bufferIdx, bool looping );
	
	//bool isPlaying();
	bool isPlaying( int bufferIdx, int sndID );
	bool handleStreamNotify( bool looped );
	
	bool checkStreamUpdate();

	bool	isStreaming() const { return mStreaming; }
	bool	isAmbient() const { return mAmbient; }

private:
	bool	internalCreate( const std::string& filename, DWORD creationFlags = 0, GUID guid3DAlgo = GUID_NULL );
	bool	restoreBuffer( IDirectSoundBuffer* buffer, bool* wasRestored );
	bool	restoreBufferAndFill( IDirectSoundBuffer* buffer, bool repeatIfLargerBuffer );

	bool fillBufferWithSound( IDirectSoundBuffer* buffer, bool repeatIfLargerBuffer );
	
	int		getFreeBufferIndex();
	long	get3DBuffer( int index, IDirectSound3DBuffer** buffer3D );


	bool isInside( DWORD pos, DWORD start, DWORD end ) const;

private:
	DECLARE_POOLED_ALLOC(dingus::CSoundResource);

private:
	// General params
	int 	mTrackCount;
	bool	mAmbient;
	bool	mStreaming;
	
	// DSound params
	IDirectSoundBuffer**	mDSBuffers; // ds buffer per track
	int*			mPlayingIDs; // playing sound IDs per track
	DWORD			mDSBufferSize;
	CSoundWaveFmt* 	mWaveFile;
	DWORD	mCreationFlags;
	
	// streaming params
	DWORD	mLastPlayPos;
	DWORD	mPlayProgress;
	DWORD	mNotifySize;
	DWORD	mTriggerWriteOffset;
	DWORD	mNextWriteOffset;
	bool	mFillNextWithSilence;
};

}; // namespace

#endif
