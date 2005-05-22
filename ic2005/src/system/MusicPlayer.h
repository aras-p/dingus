#ifndef __MUSIC_PLAYER_H
#define __MUSIC_PLAYER_H

namespace music {

void	init( HWND hwnd );
void	close();
void	update();

void	play( const char* fileName, bool loop );
void	setVolume( float volume );
float	getTime();
void	setTime( float t );
float	getLength();

};


#endif
