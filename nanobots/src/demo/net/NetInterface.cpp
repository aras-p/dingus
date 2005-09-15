#include "stdafx.h"
#include "NetInterface.h"
#include <winsock.h>


namespace {
	CConsoleChannel& NETCONS = CConsole::getChannel("net");

	SOCKET		commSocket = 0;
	sockaddr_in	sockAddr;
	in_addr		serverAddr;
	std::string serverAddrString;

	const int MAX_RECV_BUFFER = 256 * 1024;
	unsigned char	recvBuffer[MAX_RECV_BUFFER];
};



void net::initialize( const char* serverName, int serverPort )
{
	// init WinSock
	NETCONS << "initialize winsock" << endl;
	WSADATA wd;
	if( WSAStartup( MAKEWORD(1,1), &wd ) != 0 ) {
		throw ENetException( "Initialization failed" );
	}

	// initialize socket
	NETCONS << "initialize socket" << endl;
	commSocket = INVALID_SOCKET;
	commSocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if( commSocket == INVALID_SOCKET )
		throw ENetException( "Socket creation failed" );

	int trueAsInt = 1;
	int res = ::setsockopt( commSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&trueAsInt, sizeof(trueAsInt) );
	if( res == -1 )
		throw ENetException( "Socket option set failed" );

	// resolve server address
	NETCONS << "resolve server address " << serverAddrString << endl;
	memset( &sockAddr, 0, sizeof(sockAddr) );
	memset( &serverAddr, 0, sizeof(serverAddr) );
	struct hostent* he = ::gethostbyname( serverName );
	if( !he )
		throw ENetException( "Can't find server" );
	serverAddr = *( (struct in_addr*)he->h_addr );
	serverAddrString = ::inet_ntoa( serverAddr );
	NETCONS << "server address is " << serverAddrString << endl;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = ::htons( serverPort );
	sockAddr.sin_addr = serverAddr;

	// connect
	NETCONS << "connect" << endl;
	res = ::connect( commSocket, (const sockaddr*)&sockAddr, sizeof(sockAddr) );
	if( res == -1 ) {
		throw ENetException( "Connection failed" );
	}

	// set to non-blocking mode
	NETCONS << "set socket to nonblocking mode" << endl;
	u_long trueAsUlong = 1;
	res = ::ioctlsocket( commSocket, FIONBIO, &trueAsUlong );
	assert( 0==res );
}


void net::shutdown()
{
	NETCONS << "shutdown" << endl;
	::closesocket( commSocket );
	WSACleanup();
}


bool net::isConnected()
{
	return commSocket != INVALID_SOCKET;
}


bool net::receive( const unsigned char*& data, int& size )
{
	assert( commSocket != INVALID_SOCKET );

	int bytes = ::recv( commSocket, (char*)recvBuffer, MAX_RECV_BUFFER, 0 );
	if( bytes <= 0 ) {
		size = 0;
		data = 0;

		int wsaErr = WSAGetLastError();
		if( wsaErr == WSAEWOULDBLOCK ) {
			// no data available yet
			return false;
		}
		// TBD: throw?
		NETCONS << "receive error " << wsaErr << endl;
		return false;
	}

	// we have received something
	size = bytes;
	data = recvBuffer;
	return true;
}


void net::send( const void* data, int size )
{
	assert( INVALID_SOCKET != commSocket );
	int res = ::send( commSocket, (const char*)data, size, 0 );
	if( res <= 0 ) {
		NETCONS << "sent failed: " << res << endl;
	} else if( res != size ) {
		NETCONS << "sent only " << res << " bytes instead of " << size << endl;
	}
}
