#include "stdafx.h"
#include "AudioContext.h"
#include "AudioListener.h"
#include "Sound.h"
#include "../console/Console.h"

// kind of HACK-ish
#include "../resource/SoundBundle.h" 

using namespace dingus;


//#define AUDIO_CHANNEL CConsole::getChannel("audio")


CAudioContext* dingus::G_AUDIOCTX = 0;


CAudioContext::CAudioContext( HWND wnd )
:	mOpen(false),
	mInBeginScene(false),
	mMasterVolume(1.0f),
	mMasterVolumeDirty(true),
	mCurrTime(time_value(0)),
	mMasterVolumeChangedTime(time_value(0)),
	mHWnd( wnd ), m3DListener(0), mDS(0), mLastStreamUpdateCheck(time_value(0))
	
{
	assert( !G_AUDIOCTX );
	G_AUDIOCTX = this;
}

CAudioContext::~CAudioContext()
{
	if( mOpen )
		close();
	assert(0 == m3DListener);
	assert( G_AUDIOCTX );
	G_AUDIOCTX = 0;
}

bool CAudioContext::open()
{
	close();

	HRESULT hr;
	bool ok;
	
	// initialize direct sound
	safeRelease( mDS );
	hr = DirectSoundCreate8( NULL, &mDS, NULL );
	if( FAILED(hr) ) {
		CConsole::CON_WARNING << "Failed to create DirectSound device. Sound unavailable." << endl;
		close();
		return false;
	}
	assert( mDS );
	hr = mDS->SetCooperativeLevel( mHWnd, DSSCL_PRIORITY );
	assert( SUCCEEDED(hr) );
	ok = setPrimaryBufferFmt( 2, 22050, 16 );
	assert( ok );
	
	// obtain 3D listener interface
	ok = get3DListener( &m3DListener );
	if( !ok ) {
		CConsole::CON_WARNING << "Failed to obtain 3D sound listener. Sound unavailable." << endl;
		close();
		return false;
	}
	assert( m3DListener );
	memset( &m3DProps, 0, sizeof(m3DProps) );
	m3DProps.dwSize = sizeof( m3DProps );
	m3DListener->GetAllParameters( &m3DProps );

	mOpen = true;
	return true;
}


void CAudioContext::close()
{
	safeRelease( m3DListener );
	safeRelease( mDS );

	mOpen = false;
}

/*
TBD
void CAudioContext::reset()
{
	CSoundBundle::TResourceMap::iterator it, itEnd = CSoundBundle::getInstance().getSoundsEnd();
	for( it = CSoundBundle::getInstance().getSoundsBegin(); it != itEnd; ++it ) {
		CSoundResource& snd = *it->second;
		if( snd.isPlaying() )
			snd.stop();
	}
}
*/

bool CAudioContext::beginScene( time_value t )
{
	if( !mOpen )
		return false;
	assert( !mInBeginScene );
	mCurrTime = t;
	mInBeginScene = true;
	return true;
}

void CAudioContext::endScene()
{
	if( !mOpen )
		return;
	assert( mInBeginScene );

	// handle stream updates (only every 1/20 seconds)
	time_value currTime = CSystemTimer::getInstance().getTime();
	time_value diff = currTime - mLastStreamUpdateCheck;
	if( diff.value > time_value::FREQ / 20 ) {
		mLastStreamUpdateCheck = currTime;

		CSoundBundle::TResourceMap::iterator it, itEnd = CSoundBundle::getInstance().getSoundsEnd();
		for( it = CSoundBundle::getInstance().getSoundsBegin(); it != itEnd; ++it ) {
			CSoundResource& snd = *it->second;
			if( !snd.isStreaming() )
				continue;
			if( snd.checkStreamUpdate() ) {
				// TBD - get the looping flag somehow!
				//snd.handleStreamNotify();
			}
		}
	}

	mInBeginScene = false;
}



void CAudioContext::updateListener()
{
	if( !mOpen )
		return;
	assert( m3DListener );

	const SMatrix4x4& m = mListener.transform;
	const SVector3& v = mListener.velocity;
	m3DProps.vPosition.x 	  = m.getOrigin().x;
	m3DProps.vPosition.y 	  = m.getOrigin().y;
	m3DProps.vPosition.z 	  = m.getOrigin().z;
	m3DProps.vVelocity.x 	  = v.x;
	m3DProps.vVelocity.y 	  = v.y;
	m3DProps.vVelocity.z 	  = v.z;
	m3DProps.vOrientFront.x	  = m.getAxisZ().x;
	m3DProps.vOrientFront.y	  = m.getAxisZ().y;
	m3DProps.vOrientFront.z	  = m.getAxisZ().z;
	m3DProps.vOrientTop.x	  = m.getAxisY().x;
	m3DProps.vOrientTop.y	  = m.getAxisY().y;
	m3DProps.vOrientTop.z	  = m.getAxisY().z;
	m3DProps.flDistanceFactor  = 1.0f;
	m3DProps.flRolloffFactor   = mListener.rolloffFactor;
	m3DProps.flDopplerFactor   = mListener.dopplerFactor;
	
	HRESULT hr = m3DListener->SetAllParameters( &m3DProps, DS3D_IMMEDIATE );
	assert( SUCCEEDED(hr) );
}

void CAudioContext::startSound( CSound& sound )
{
	if( !mOpen )
		return;
	sound.start();
}

void CAudioContext::updateSound( CSound& sound )
{
	if( !mOpen )
		return;
	sound.update();
}

void CAudioContext::stopSound( CSound& sound )
{
	if( !mOpen )
		return;
	sound.stop();
}

bool CAudioContext::setPrimaryBufferFmt( DWORD channels, DWORD freq, DWORD bitrate )
{
	HRESULT hr;
	IDirectSoundBuffer* primaryBuf = NULL;
	
	assert(0 != mDS);
	
	// Get the primary buffer 
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize 	   = sizeof(DSBUFFERDESC);
	dsbd.dwFlags	   = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;
	
	if( FAILED( hr = mDS->CreateSoundBuffer( &dsbd, &primaryBuf, NULL ) ) )
		return false;
	
	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag		= (WORD)WAVE_FORMAT_PCM; 
	wfx.nChannels		= (WORD)channels; 
	wfx.nSamplesPerSec	= (DWORD)freq; 
	wfx.wBitsPerSample	= (WORD)bitrate; 
	wfx.nBlockAlign 	= (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD)(wfx.nSamplesPerSec * wfx.nBlockAlign);
	if( FAILED( hr = primaryBuf->SetFormat(&wfx) ) )
		return false;
	
	if( primaryBuf )
		primaryBuf->Release();
	
	return true;
}


bool CAudioContext::get3DListener( IDirectSound3DListener** listener )
{
	HRESULT hr;
	DSBUFFERDESC dsbdesc;
	IDirectSoundBuffer* primaryBuf = NULL;
	
	assert(0 != listener);
	assert(0 != mDS);
	
	*listener = NULL;
	
	// Obtain primary buffer, asking it for 3D control
	ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	if( FAILED( hr = mDS->CreateSoundBuffer( &dsbdesc, &primaryBuf, NULL ) ) )
		return false;
	
	if( FAILED( hr = primaryBuf->QueryInterface( IID_IDirectSound3DListener, (void**)listener ) ) ) {
		if( primaryBuf ) primaryBuf->Release();
		return false;
	}
	
	if( primaryBuf ) primaryBuf->Release();

	return true;
}
