#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

#include "../game/GameTypes.h"

class CGameDesc;
class CGameState;
struct SServerState;

namespace net {


	CGameDesc* receiveGameDesc( std::string& errMsg );

	void receiveServerState( int playerCount, SServerState& state, std::string& errMsg, bool startFlag );

	bool requestJoin( int playerID );

	void updateGame( int keyCode, int locX, int locY, CGameState& state );

};


#endif
