#include "stdafx.h"
#include "NetMessages.h"
#include "NetInterface.h"
#include "../game/GameDesc.h"
#include "../ByteUtils.h"

using namespace net;

/*
Message types:
2 = TestConnection
3 = SendGameDesc
5 = ??? (len=2, 5 0)
6 = Looks like GameData (len=33, 20 23 38 201 65 136 199 8 0 0 2 0 0 0 9 108 111 103 80 105 101 114 114 101 1 0 0 0 0 0 0 0)
*/

enum eMessage {
	NMSG_ERROR = 0,
	NMSG_OK = 1,
	NMSG_TEST_CONN = 2,
	NMSG_GAME_DESC = 3,
	NMSG_SERVER_STATE = 5,
};

const int NETWORK_PROTOCOL_VER = 1;


/*
static void gFetchResponse()
{
	NETCONS << "Receive response..." << endl;
	::Sleep( 100 );

	BYTE* resp;
	int respSize;
	bool have = net::receive( resp, respSize );
	if( have && respSize > 0 ) {
		NETCONS << "len=" << respSize << " type=" << resp[0] << endl;
		for( int i = 1; i < respSize; ++i ) {
			NETCONS << resp[i] << '[' << ((resp[i]>=' ' && resp[i]<=127) ? (char)resp[i] : '?') << "] ";
			if( i % 20 == 0 && i != respSize-1 ) {
				NETCONS << endl;
			}
		}
		NETCONS << endl;
	}
}

static void gTestStuff()
{
	int MSGS[] = { 2, 3, 6 };
	int COUNT = sizeof(MSGS) / sizeof(MSGS[0]);

	BYTE msg[4];
	memset( &msg, 0, sizeof(msg) );
	for( int i = 0; i < COUNT; ++i ) {
		msg[0] = MSGS[i];
		NETCONS << "Send msg type " << msg[0] << endl;
		net::send( &msg, 1 );
		gFetchResponse();
	}
}
*/

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


net::SServerState::SServerState()
{
	memset( this, 0, sizeof(this) );
}

void net::receiveServerState( int playerCount, SServerState& state, std::string& errMsg )
{
	// query server state
	NETCONS << "Query server state" << endl;
	BYTE msg;
	msg = NMSG_SERVER_STATE;
	net::send( &msg, sizeof(msg) );

	// receive server state
	state = SServerState();
	BYTE msgType = bu::receiveByte();
	if( msgType != NMSG_SERVER_STATE ) {
		errMsg = "Failed to query server state";
		return;
	}
	BYTE stateByte = bu::receiveByte();
	if( stateByte > GST_ENDED ) {
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
