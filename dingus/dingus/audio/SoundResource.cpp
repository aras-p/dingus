#include "stdafx.h"
#include "SoundResource.h"
#include "SoundWaveFmt.h"
#include "AudioContext.h"
//#include "../console/Console.h"

using namespace dingus;

DEFINE_POOLED_ALLOC(dingus::CSoundResource,64,false);

CSoundResource::CSoundResource()
:	mTrackCount(5), mAmbient(false), mStreaming(false),
	mDSBuffers(0), mPlayingIDs(0), mWaveFile(0)
{
}


CSoundResource::~CSoundResource()
{
	// TBD
}



bool CSoundResource::internalCreate( const std::string& filename, DWORD creationFlags, GUID guid3DAlgo )
{
	HRESULT hr;
	int i;
	
	assert( G_AUDIOCTX );
	assert( !filename.empty() );
	assert( mStreaming && mTrackCount==1 || !mStreaming && mTrackCount > 1 );
	assert( !mDSBuffers );
	assert( !mWaveFile );

	mDSBufferSize = 0;
	mWaveFile = 0;
	mCreationFlags = creationFlags;

	mPlayingIDs = new int[mTrackCount];
	if( mPlayingIDs == NULL )
		goto _error;
	memset( mPlayingIDs, -1, mTrackCount*sizeof(mPlayingIDs[0]) );
	
	mDSBuffers = new IDirectSoundBuffer*[mTrackCount];
	if( mDSBuffers == NULL )
		goto _error;
	
	mWaveFile = new CSoundWaveFmt();
	if( mWaveFile == NULL )
		goto _error;
	
	mWaveFile->open( filename, NULL, true );
	if( mWaveFile->getSize() == 0 ) // Wave is blank, so don't create it.
		goto _error;
	
	// figure out the size of dsound buffers
	if( !mStreaming ) {
		mDSBufferSize = mWaveFile->getSize();
	} else {
		// 64KB streaming buffer
		int numNotifications = 2;
		int blockSize = (1<<18) / numNotifications;
		mDSBufferSize = numNotifications * blockSize;
		// streaming members
		mLastPlayPos = 0;
		mPlayProgress = 0;
		mNotifySize = blockSize;
		mTriggerWriteOffset = 0;
		mNextWriteOffset = 0;
		mFillNextWithSilence = false;
	}

	
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize 		 = sizeof(DSBUFFERDESC);
	dsbd.dwFlags		 = creationFlags;
	if( mStreaming )
		dsbd.dwFlags |= DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes	 = mDSBufferSize;
	dsbd.guid3DAlgorithm = guid3DAlgo;
	dsbd.lpwfxFormat	 = mWaveFile->mFormat;
	
	hr = G_AUDIOCTX->getDS().CreateSoundBuffer( &dsbd, &mDSBuffers[0], NULL );
	if( hr == DS_NO_VIRTUALIZATION ) {
		assert( false );
	}
	if( FAILED(hr) ) {
		// TBD
		//n_printf("CreateSoundBuffer: Wave file must be mono for 3D control. or using mAmbient sound setting by 2D sound.\n");
		goto _error;
	}
	
	if( (creationFlags & DSBCAPS_CTRLFX) == 0 ) {
		for( i = 1; i < mTrackCount; ++i ) {
			if( FAILED( hr = G_AUDIOCTX->getDS().DuplicateSoundBuffer( mDSBuffers[0], &mDSBuffers[i] ) ) ) {
				//n_printf("DuplicateSoundBuffer"); // TBD
				goto _error;
			}
		}
	} else {
		for( i = 1; i < mTrackCount; ++i ) {
			hr = G_AUDIOCTX->getDS().CreateSoundBuffer( &dsbd, &mDSBuffers[i], NULL );
			if( FAILED(hr) ) {
				//n_printf("CreateSoundBuffer"); // TBD
				goto _error;
			}
		}
	}

	fillBufferWithSound( mDSBuffers[0], false );
	return true;
	
_error:
	// cleanup
	safeDelete( mWaveFile );
	safeDeleteArray( mDSBuffers );
	safeDeleteArray( mPlayingIDs );
	return false;
}



bool CSoundResource::createResource( const std::string& fileName, int tracks, bool ambient, bool streaming )
{
	assert( !mDSBuffers );

	mTrackCount = tracks;
	mAmbient = ambient;
	mStreaming = streaming;
	
	DWORD creationFlags = DSBCAPS_CTRLVOLUME /*| DSBCAPS_LOCDEFER*/;
	creationFlags |= DSBCAPS_LOCSOFTWARE;
	if( !mAmbient )
		creationFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;

	if( internalCreate( fileName, creationFlags, DS3DALG_DEFAULT ) == false ) {
		// TBD
		//n_printf("nDSoundServer::LoadResource(): Creating static sound '%s' failed!\n", mangledPath.Get());
		return false;
	}

	assert( mDSBuffers );
	return true;
}


void CSoundResource::deleteResource()
{
	if( !G_AUDIOCTX->isOpen() )
		return;
	
	assert( mDSBuffers );
	assert( mPlayingIDs );
	for( int i = 0; i < mTrackCount; ++i ) {
		mDSBuffers[i]->Release();
	}
	delete[] mDSBuffers;
	delete[] mPlayingIDs;
	safeDelete( mWaveFile );
	mPlayingIDs = 0;
	mDSBuffers = 0;
}

/*
void CSoundResource::stop()
{
	if( mDSBuffers == NULL )
		return;
	HRESULT hr = 0;
	for( int i=0; i<mTrackCount; ++i )
		hr |= mDSBuffers[i]->Stop();
}
*/
void CSoundResource::stop( int bufferIdx, int sndID )
{
	if( bufferIdx < 0 )
		return;
	assert( bufferIdx < mTrackCount );
	if( mPlayingIDs[bufferIdx] != sndID )
		return; // already playing other sound - don't stop
	HRESULT hr = mDSBuffers[bufferIdx]->Stop();
}

void CSoundResource::setVolume( int bufferIdx, int sndID, LONG volume )
{
	if( bufferIdx < 0 )
		return;
	assert( bufferIdx < mTrackCount );
	if( mPlayingIDs[bufferIdx] != sndID )
		return; // already playing other sound - don't set volume
	HRESULT hr = mDSBuffers[bufferIdx]->SetVolume( volume );
}


bool CSoundResource::fillBufferWithSound( IDirectSoundBuffer* buffer, bool repeatIfLargerBuffer )
{
	HRESULT hr; 
	void*	lockedBuffer = NULL; // Pointer to locked buffer memory
	DWORD	lockedSize = 0; // Size of the locked DirectSound buffer
	DWORD	dataRead = 0; // Amount of data read from the wav file 
	
	assert( 0 != buffer );
	
	// Make sure we have focus, and we didn't just switch in from
	// an app which had a dsound device
	hr = restoreBuffer( buffer, NULL );
	assert(hr);
	
	// lock the buffer
	if( FAILED( hr = buffer->Lock( 0, mDSBufferSize, &lockedBuffer, &lockedSize, NULL, NULL, 0L ) ) )
		assert(hr);
	
	// reset the file to the beginning 
	mWaveFile->resetFile();
	
	hr = mWaveFile->read( (BYTE*)lockedBuffer, lockedSize, &dataRead );
	assert(hr);
	
	if( dataRead == 0 ) {
		// wav is blank, so just fill with silence
		FillMemory( (BYTE*)lockedBuffer, lockedSize, (BYTE)(mWaveFile->mFormat->wBitsPerSample == 8 ? 128 : 0 ) );

	} else if( dataRead < lockedSize ) {
		// if the file was smaller than the buffer, we need to fill the remainder
		if( repeatIfLargerBuffer ) {		
			// reset the file and fill the buffer with wav data
			DWORD readSoFar = dataRead;
			while( readSoFar < lockedSize ) {  
				hr = mWaveFile->resetFile();
				assert(hr);
				hr = mWaveFile->read( (BYTE*)lockedBuffer + readSoFar, lockedSize - readSoFar, &dataRead );
				assert(hr);
				readSoFar += dataRead;
			} 
		} else 	{
			// just fill in silence 
			FillMemory( (BYTE*)lockedBuffer + dataRead, lockedSize - dataRead, (BYTE)(mWaveFile->mFormat->wBitsPerSample == 8 ? 128 : 0 ) );
		}
	}
	
	buffer->Unlock( lockedBuffer, lockedSize, NULL, 0 );
	return true;
}

/**
 * Restores the lost buffer.
 * (*wasRestored) returns true if the buffer was restored. It can also NULL if the information is not needed.
 */
bool CSoundResource::restoreBuffer( IDirectSoundBuffer* buffer, bool* wasRestored )
{
	HRESULT hr;
	
	assert(0 != buffer);
	if( wasRestored )
		*wasRestored = false;
	
	DWORD status;
	if( FAILED( hr = buffer->GetStatus( &status ) ) )
		return false;
	
	if( status & DSBSTATUS_BUFFERLOST ) {
		// Since the app could have just been activated, then dsound may not
		// be giving us control yet, so the restoring of the buffer may fail.  
		// If it does, sleep until dsound gives us control.
		do {
			hr = buffer->Restore();
			if( hr == DSERR_BUFFERLOST )
				Sleep( 10 );
		} while( ( hr = buffer->Restore() ) == DSERR_BUFFERLOST );
		
		if( wasRestored != NULL )
			*wasRestored = true;
	}
	return true;
}

bool CSoundResource::restoreBufferAndFill( IDirectSoundBuffer* buffer, bool repeatIfLargerBuffer )
{
	HRESULT hr;
	bool restored;
	if( FAILED( hr = restoreBuffer( buffer, &restored ) ) )
		return false;
	if( restored ) {
		if( FAILED( hr = fillBufferWithSound( buffer, repeatIfLargerBuffer ) ) )
			return false;
	}
	return true;
}

long CSoundResource::get3DBuffer( int index, IDirectSound3DBuffer** buffer3D )
{
	if( mDSBuffers == NULL )
		return false;
	if( index >= mTrackCount )
		return false;
	
	*buffer3D = NULL;
	return mDSBuffers[index]->QueryInterface( IID_IDirectSound3DBuffer, (void**)buffer3D );
}

int CSoundResource::getFreeBufferIndex()
{
	assert( mDSBuffers );

	// try to find free buffer
	for( int i = 0; i < mTrackCount; ++i ) {
		assert( mDSBuffers[i] );
		DWORD status = 0;
		mDSBuffers[i]->GetStatus( &status );
		if( ( status & DSBSTATUS_PLAYING ) == 0 ) {
			mDSBuffers[i]->SetCurrentPosition( 0L );
			return i;
		}
	}
	
	// or return random one
	int index = rand() % mTrackCount;
	mDSBuffers[index]->SetCurrentPosition( 0L );
	return index;
}

int CSoundResource::play( int sndID, DWORD priority, DWORD flags, LONG volume, LONG freq, LONG pan )
{
	assert( mDSBuffers );

	int bufferIdx = getFreeBufferIndex();
	IDirectSoundBuffer* buffer = mDSBuffers[bufferIdx];
	if( !restoreBufferAndFill( buffer, false ) )
		return -1;
	
	if( mCreationFlags & DSBCAPS_CTRLVOLUME )
		buffer->SetVolume( volume );
	if( freq != -1 && (mCreationFlags & DSBCAPS_CTRLFREQUENCY) )
		buffer->SetFrequency( freq );
	if( mCreationFlags & DSBCAPS_CTRLPAN )
		buffer->SetPan( pan );
	
	mPlayingIDs[bufferIdx] = sndID;
	buffer->Play( 0, priority, flags );
	return bufferIdx;
}


int CSoundResource::play3D( int sndID, const DS3DBUFFER& props3D, DWORD priority, DWORD flags, LONG volume, LONG freq )
{
	HRESULT hr;
	DWORD	baseFreq;
	
	assert( mDSBuffers );

	// cull the sound if too far
	const SVector3& earPos = G_AUDIOCTX->getListener().transform.getOrigin();
	SVector3 toear( earPos.x - props3D.vPosition.x, earPos.y - props3D.vPosition.y, earPos.z - props3D.vPosition.z );
	if( toear.lengthSq() >= props3D.flMaxDistance * props3D.flMaxDistance )
		return 0;

	
	
	int bufferIdx = getFreeBufferIndex();
	IDirectSoundBuffer* buffer = mDSBuffers[bufferIdx];
	if( !restoreBufferAndFill( buffer, false ) )
		return -1;
	
	if( mCreationFlags & DSBCAPS_CTRLVOLUME )
		buffer->SetVolume( volume );
	if( mCreationFlags & DSBCAPS_CTRLFREQUENCY ) {
		buffer->GetFrequency( &baseFreq );
		buffer->SetFrequency( baseFreq + freq );
	}
	
	IDirectSound3DBuffer* buf3D;
	hr = buffer->QueryInterface( IID_IDirectSound3DBuffer, (void**)&buf3D );
	if( SUCCEEDED( hr ) ) {
		hr = buf3D->SetAllParameters( &props3D, DS3D_IMMEDIATE );
		if( SUCCEEDED( hr ) ) {
			mPlayingIDs[bufferIdx] = sndID;
			hr = buffer->Play( 0, priority, flags );
		}
		buf3D->Release();
	}
	
	return bufferIdx;
}

bool CSoundResource::reset( int bufferIdx, bool looping )
{
	assert( mDSBuffers );
	assert( mWaveFile );
	assert( bufferIdx >= 0 && bufferIdx < mTrackCount );
	if( mStreaming ) {
		mLastPlayPos = 0;
		mPlayProgress = 0;
		mNextWriteOffset = 0;
		mFillNextWithSilence = false;
		
		if( !restoreBufferAndFill( mDSBuffers[bufferIdx], false ) )
			return false;
		mWaveFile->resetFile();
		mDSBuffers[bufferIdx]->SetCurrentPosition( 0L );
		fillBufferWithSound( mDSBuffers[bufferIdx], looping );

	} else {
		mDSBuffers[bufferIdx]->SetCurrentPosition( 0L );
	}
	return true;
}

/*
bool CSoundResource::isPlaying()
{
	if( mDSBuffers == NULL )
		return false;
	
	for( int i = 0; i < mTrackCount; ++i ) {
		if( mDSBuffers[i] ) {  
			DWORD status = 0;
			mDSBuffers[i]->GetStatus( &status );
			if( status & DSBSTATUS_PLAYING )
				return true;
		}
	}

	return false;
}
*/

bool CSoundResource::isPlaying( int bufferIdx, int sndID )
{
	if( bufferIdx < 0 )
		return false;
	assert( bufferIdx < mTrackCount );
	if( mPlayingIDs[bufferIdx] != sndID )
		return false;
	DWORD status = 0;
	mDSBuffers[bufferIdx]->GetStatus( &status );
	if( status & DSBSTATUS_PLAYING )
		return true;
	return false;
}

bool CSoundResource::handleStreamNotify( bool looped )
{
	HRESULT hr;
	DWORD	currPlayPos;
	DWORD	playDelta;
	DWORD	bytesWritten;
	void*	lockedBuffer = NULL;
	void*	lockedBuffer2 = NULL;
	DWORD	lockedSize;
	DWORD	lockedSize2;
	
	if( mDSBuffers == NULL || mWaveFile == NULL )
		return false;
	
	// Restore the buffer if it was lost
	if( !restoreBufferAndFill( mDSBuffers[0], false ) )
		return false;
	
	if( FAILED( hr = mDSBuffers[0]->Lock( mNextWriteOffset, mNotifySize, 
		&lockedBuffer, &lockedSize, 
		&lockedBuffer2, &lockedSize2, 0L ) ) )
		return false;
	
	// mDSBufferSize and mNextWriteOffset are both multiples of mNotifySize, 
	// it should the second buffer, so it should never be valid
	if( lockedBuffer2 != NULL )
		return false; 
	
	if( !mFillNextWithSilence ) {
		// Fill the buffer with wav data
		if( FAILED( hr = mWaveFile->read( (BYTE*) lockedBuffer, lockedSize, &bytesWritten ) ) )
			return false;
	} else {
		// Fill the buffer with silence
		FillMemory( lockedBuffer, lockedSize, (BYTE)( mWaveFile->mFormat->wBitsPerSample == 8 ? 128 : 0 ) );
		bytesWritten = lockedSize;
	}
	
	// If the number of bytes written is less than the amount we requested, we have a short file.
	if( bytesWritten < lockedSize ) {
		if( !looped )  {
			// Fill in silence for the rest of the buffer.
			FillMemory( (BYTE*) lockedBuffer + bytesWritten, lockedSize - bytesWritten, (BYTE)(mWaveFile->mFormat->wBitsPerSample == 8 ? 128 : 0 ) );
			// Any future notifications should just fill the buffer with silence
			mFillNextWithSilence = true;

		} else {
			// We are looping, so reset the file and fill the buffer with wav data
			DWORD readSoFar = bytesWritten; 
			while( readSoFar < lockedSize ) {  
				if( FAILED( hr = mWaveFile->resetFile() ) )
					return false;
				if( FAILED( hr = mWaveFile->read( (BYTE*)lockedBuffer + readSoFar,
					lockedSize - readSoFar,
					&bytesWritten ) ) )
					return false;
				readSoFar += bytesWritten;
			} 
		} 
	}
	
	mDSBuffers[0]->Unlock( lockedBuffer, lockedSize, NULL, 0 );
	
	// Figure out how much data has been played so far.  When we have played
	// past the end of the file, we will either need to start filling the
	// buffer with silence or starting reading from the beginning of the file, 
	// depending if the user wants to loop the sound
	if( FAILED( hr = mDSBuffers[0]->GetCurrentPosition( &currPlayPos, NULL ) ) )
		return false;
	
	// Check to see if the position counter looped
	if( currPlayPos < mLastPlayPos )
		playDelta = ( mDSBufferSize - mLastPlayPos ) + currPlayPos;
	else
		playDelta = currPlayPos - mLastPlayPos;
	
	mPlayProgress += playDelta;
	mLastPlayPos = currPlayPos;
	
	// If we are now filling the buffer with silence, then we have found the end so 
	// check to see if the entire sound has played, if it has then stop the buffer.
	if( mFillNextWithSilence ) {
		// We don't want to cut off the sound before it's done playing.
		if( mPlayProgress >= mWaveFile->getSize() )
			mDSBuffers[0]->Stop();
	}
	
	// Update where the buffer will lock for next time
	mNextWriteOffset += lockedSize; 
	mTriggerWriteOffset = mNextWriteOffset - mNotifySize;
	mNextWriteOffset %= mDSBufferSize; // Circular buffer
	mTriggerWriteOffset %= mDSBufferSize;
	
	return true;
}

bool CSoundResource::isInside(DWORD pos, DWORD start, DWORD end) const
{
	if( start < end )
		return ((pos >= start) && (pos < end));
	else
		return ((pos >= start) || (pos < end));
}

bool CSoundResource::checkStreamUpdate()
{
	assert( mDSBuffers );
	// check if playing
	DWORD status = 0;
	mDSBuffers[0]->GetStatus( &status );
	if( !(status & DSBSTATUS_PLAYING) )
		return false;

	// get current write cursor position
	DWORD playCursor;
	DWORD writeCursor;
	HRESULT hr = mDSBuffers[0]->GetCurrentPosition( &playCursor, &writeCursor );
	return isInside( writeCursor, mTriggerWriteOffset, mNextWriteOffset );
}
