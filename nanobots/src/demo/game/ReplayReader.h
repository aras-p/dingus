#ifndef __REPLAY_READER_H
#define __REPLAY_READER_H


class CGameReplay;

/// @return true on success.
bool	gReadReplay( const char* fileName, CGameReplay& game );


#endif
