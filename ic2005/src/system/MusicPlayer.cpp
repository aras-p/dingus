#include "stdafx.h"
#include "MusicPlayer.h"

#define USE_BASS


// ---- BASS ----------------------------------------------------------------

#ifdef USE_BASS

#include "../extern/bass.h"

static HSTREAM	gSoundStream = 0;
static float	gSoundLength = 0.0f;


void music::init( HWND hwnd )
{
	gSoundStream = NULL;
	//assert( FSOUND_GetVersion() >= FMOD_VERSION );
	//FSOUND_SetDriver(0);
	//char c = FSOUND_Init( 44100, 16, 0 );
	//assert( c );
	if( !BASS_Init( 1, 44100, 0, hwnd, NULL ) ) {
		BASS_Init( 0, 44100, 0, hwnd, NULL );
	}
}
void music::close()
{
	BASS_Free();
}
void music::play( const char* fileName, bool loop )
{
	if( gSoundStream ) {
		BASS_StreamFree( gSoundStream );
		gSoundStream = NULL;
	}
	gSoundStream = BASS_StreamCreateFile( FALSE, fileName, 0, 0, loop ? BASS_SAMPLE_LOOP : 0 );
	if( !gSoundStream )
		return;

	QWORD len = BASS_StreamGetLength( gSoundStream ); // length in bytes
	gSoundLength = BASS_ChannelBytes2Seconds( gSoundStream, len ); // the time length

	BASS_ChannelPlay( gSoundStream, FALSE );
}
float music::getTime()
{
	if( !gSoundStream )
		return 0.0f;
	QWORD pos = BASS_ChannelGetPosition( gSoundStream ); // pos in bytes
	return BASS_ChannelBytes2Seconds( gSoundStream, pos ); // seconds
}
float music::getLength()
{
	if( !gSoundStream )
		return 0.0f;
	return gSoundLength;
}

void music::update()
{
}


#endif // USE_BASS
