#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

#include "../game/GameTypes.h"

class CGameDesc;
class CGameState;

namespace net {


	struct SServerState {
		SServerState();
		
		eGameServerState	state;
		bool	playerRealtime[G_MAX_PLAYERS]; // first is dummy (AI)
		bool	playerControlled[G_MAX_PLAYERS]; // first is dummy (AI)
	};

	CGameDesc* receiveGameDesc( std::string& errMsg );

	void receiveServerState( int playerCount, SServerState& state, std::string& errMsg, bool startFlag );

	bool requestJoin( int playerID );

	void updateGame( int keyCode, int locX, int locY, CGameState& state );

};


#endif
