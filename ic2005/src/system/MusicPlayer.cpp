#include "stdafx.h"
#include "MusicPlayer.h"

#define USE_BASS


// ---- BASS ----------------------------------------------------------------

#ifdef USE_BASS

#include "../extern/bass.h"

static HSTREAM	gSoundStream = 0;
static float	gSoundLength = 0.0f;
static bool		gSoundLooped = false;


void music::init( HWND hwnd )
{
	gSoundStream = NULL;
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
	gSoundLooped = false;
	gSoundStream = BASS_StreamCreateFile( FALSE, fileName, 0, 0, loop ? BASS_SAMPLE_LOOP : 0 );
	if( !gSoundStream )
		return;

	gSoundLooped = loop;

	QWORD len = BASS_StreamGetLength( gSoundStream ); // length in bytes
	gSoundLength = BASS_ChannelBytes2Seconds( gSoundStream, len ); // the time length

	BASS_ChannelPlay( gSoundStream, FALSE );
	setVolume( 0.0f );
}

float music::getTime()
{
	if( !gSoundStream )
		return 0.0f;
	QWORD pos = BASS_ChannelGetPosition( gSoundStream ); // pos in bytes
	return BASS_ChannelBytes2Seconds( gSoundStream, pos ); // seconds
}
void music::setTime( float t )
{
	if( !gSoundStream )
		return;
	QWORD pos = BASS_ChannelSeconds2Bytes( gSoundStream, t );
	BASS_ChannelSetPosition( gSoundStream, pos );
}
float music::getLength()
{
	if( !gSoundStream )
		return 0.0f;
	return gSoundLength;
}


void music::update()
{
	// if we're playing looped music, fade in/out at ends
	float volume = 1.0f;
	if( gSoundLooped ) {
		const float LOOP_FADE_TIME = 0.5f;

		float t = getTime();

		if( t <= LOOP_FADE_TIME ) {
			volume = t / LOOP_FADE_TIME;
		} else if( t >= getLength()-LOOP_FADE_TIME ) {
			volume = (getLength() - t) / LOOP_FADE_TIME;
		}
	}
	setVolume( volume );
}

void music::setVolume( float volume )
{
	if( !gSoundStream )
		return;
	BASS_ChannelSetAttributes( gSoundStream, -1, volume*100, -101 );
}



#endif // USE_BASS
