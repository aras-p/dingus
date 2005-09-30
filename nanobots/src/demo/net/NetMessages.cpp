#include "stdafx.h"
#include "NetMessages.h"
#include "NetInterface.h"
#include "../game/GameDesc.h"
#include "../game/GameState.h"
#include "../ByteUtils.h"

using namespace net;


enum eMessage {
	NMSG_ERROR = 0,
	NMSG_OK = 1,
	NMSG_TEST_CONN = 2,
	NMSG_GAME_DESC = 3,
	NMSG_JOIN = 4,
	NMSG_SERVER_STATE = 5,
	NMSG_GAME_DATA = 6,
	NMSG_START = 7,
	NMSG_GAME_ENDED = 7,
};

const int NETWORK_PROTOCOL_VER = 1;



CGameDesc* net::receiveGameDesc( std::string& errMsg )
{
	// test connection protocol
	NETCONS << "Connection protocol test" << endl;
	BYTE msg;
	msg = NMSG_TEST_CONN;
	net::send( &msg, sizeof(msg) );

	const BYTE* data;
	bool got = net::receiveChunk( data, 2, true );
	if( data[0] != NMSG_TEST_CONN || data[1] != NETWORK_PROTOCOL_VER ) {
		errMsg = "Incorrect client/server protocol version";
		return NULL;
	}

	// get game desc
	NETCONS << "Fetch game description" << endl;
	msg = NMSG_GAME_DESC;
	net::send( &msg, sizeof(msg) );
	net::receiveChunk( data, 1, true );
	if( data[0] != NMSG_GAME_DESC ) {
		errMsg = "Failed to get game description";
		return NULL;
	}

	// initialize game desc
	CGameDesc* gdesc = new CGameDesc();
	errMsg = gdesc->initialize();
	if( !errMsg.empty() ) {
		delete gdesc;
		return NULL;
	}

	errMsg = "";
	return gdesc;
}


void net::receiveServerState( int playerCount, SServerState& state, std::string& errMsg, bool startFlag )
{
	// query server state
	BYTE msg;
	if( startFlag ) {
		NETCONS << "Start game" << endl;
		msg = NMSG_START;
	} else {
		NETCONS << "Query server state" << endl;
		msg = NMSG_SERVER_STATE;
	}
	net::send( &msg, sizeof(msg) );

	// receive server state
	state = SServerState();
	BYTE msgType = bu::receiveByte();
	if( msgType != NMSG_SERVER_STATE ) {
		errMsg = "Failed to receive server state";
		return;
	}
	BYTE stateByte = bu::receiveByte();
	if( stateByte > GST_READYTOSTART ) {
		errMsg = "Invalid game server state";
		return;
	}
	state.state = (eGameServerState)stateByte;
	if( state.state == GST_STARTING || state.state == GST_STARTED ) {
		// skip ticks
		BYTE* data;
		net::receiveChunk( data, 8, true );
	}
	// read player states, skip first (it's AI)
	for( int i = 1; i < playerCount; ++i ) {
		BYTE realtime = bu::receiveByte();
		state.playerRealtime[i] = realtime ? true : false;
		if( realtime ) {
			BYTE ctrl = bu::receiveByte();
			state.playerControlled[i] = ctrl ? true : false;
		}
	}
	
	errMsg = "";
}


bool net::requestJoin( int playerID )
{
	NETCONS << "Request join for player" << playerID << endl;
	BYTE msg[2];
	msg[0] = NMSG_JOIN;
	msg[1] = playerID;
	net::send( &msg, sizeof(msg) );

	// receive join or error
	BYTE msgType = bu::receiveByte();
	if( msgType == NMSG_JOIN ) {
		NETCONS << "Join accepted" << endl;
		return true;
	} else if( msgType == NMSG_ERROR ) {
		NETCONS << "Join not accepted" << endl;
		return false;
	} else {
		NETCONS << "ERROR: unrecognized join response" << endl;
		return false;
	}
}

void net::updateGame( int keyCode, int locX, int locY, CGameState& state )
{
	//NETCONS << "Update game" << endl;
	BYTE msg[4];
	msg[0] = NMSG_GAME_DATA;
	msg[1] = keyCode;
	msg[2] = locX;
	msg[3] = locY;
	net::send( &msg, sizeof(msg) );

	// receive
	BYTE msgType = bu::receiveByte();
	if( msgType == NMSG_GAME_DATA ) {
		state.updateState();
	} else if( msgType == NMSG_GAME_ENDED ) {
		// TBD: game ended!
	}
}
