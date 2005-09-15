#include "stdafx.h"
#include "NetMessages.h"
#include "NetInterface.h"


using namespace net;

/*
Message types:
2 = TestConnection
3 = SendGameState
5 = ??? (len=2, 5 0)
6 = Looks like GameData (len=33, 20 23 38 201 65 136 199 8 0 0 2 0 0 0 9 108 111 103 80 105 101 114 114 101 1 0 0 0 0 0 0 0)
*/

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

void net::testConn()
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
