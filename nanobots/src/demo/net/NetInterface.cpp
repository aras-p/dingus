#include "stdafx.h"
#include "NetInterface.h"
#include <winsock2.h>


CConsoleChannel& net::NETCONS = CConsole::getChannel("net");

namespace {

	SOCKET		commSocket = INVALID_SOCKET;
	sockaddr_in	sockAddr;
	in_addr		serverAddr;
	std::string serverAddrString;

	const int MAX_RECV_BUFFER = 256 * 1024;
	unsigned char	recvBuffer[MAX_RECV_BUFFER];

	bool	hasDataToRecv = false;

	bool	chunkReceiving = false;
	int		chunkTotalSize;
	int		chunkReceivedSize;
	int		chunkCounter;
};



void net::initialize( const char* serverName, int serverPort, HWND wnd )
{
	// init WinSock
	NETCONS << "initialize winsock" << endl;
	WSADATA wd;
	if( WSAStartup( MAKEWORD(2,0), &wd ) != 0 ) {
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
	//u_long trueAsUlong = 1;
	//res = ::ioctlsocket( commSocket, FIONBIO, &trueAsUlong );
	//assert( 0==res );
	res = WSAAsyncSelect( commSocket, wnd, NET_ASYNC_MESSAGE, FD_READ );
	if( res == -1 ) {
		throw ENetException( "Failed to set socket to nonblocking mode" );
	}

	hasDataToRecv = false;
	chunkReceiving = false;
	chunkReceivedSize = 0;
	chunkTotalSize = 0;
	chunkCounter = 0;
}


void net::shutdown()
{
	NETCONS << "shutdown" << endl;
	::shutdown( commSocket, SD_SEND );
	::closesocket( commSocket );
	WSACleanup();
}


bool net::isConnected()
{
	return commSocket != INVALID_SOCKET;
}


void net::onAsyncMsg( WPARAM wparam, LPARAM lparam )
{
	int event = WSAGETSELECTEVENT( lparam );
	int err = WSAGETSELECTERROR( lparam );
	switch( event ) {
	case FD_READ:
		hasDataToRecv = true;
		break;
	}
}


bool net::receiveChunk( const unsigned char*& data, int reqSize, bool wait )
{
	assert( commSocket != INVALID_SOCKET );
	assert( reqSize < MAX_RECV_BUFFER );
	data = NULL;

	if( reqSize == 0 )
		return true;

	// no data available yet
	if( !hasDataToRecv && !wait )
		return false;

_spinwait:
	
	// receive as much bytes as we still need
	int bytesWanted;
	if( chunkReceiving ) {
		assert( reqSize == chunkTotalSize );
		bytesWanted = chunkTotalSize - chunkReceivedSize;
	} else {
		bytesWanted = reqSize;
		chunkTotalSize = reqSize;
		chunkReceivedSize = 0;
	}
	assert( bytesWanted + chunkReceivedSize < MAX_RECV_BUFFER );

	int bytes = ::recv( commSocket, (char*)recvBuffer+chunkReceivedSize, bytesWanted, 0 );
	hasDataToRecv = false;

	// check for errors
	if( bytes == 0 ) {
		if( wait )
			goto _spinwait;
		return false;
	}
	if( bytes < 0 ) {
		int wsaErr = WSAGetLastError();
		if( wsaErr == WSAEWOULDBLOCK ) {
			// no data available yet
			if( wait )
				goto _spinwait;
			return false;
		}
		// TBD: throw?
		NETCONS << "receive() error " << wsaErr << endl;
		return false;
	}

	// we've received something!
	chunkReceivedSize += bytes;

	assert( chunkReceivedSize <= reqSize );
	if( chunkReceivedSize == reqSize ) {
		// finished whole chunk
		chunkReceiving = false;
		chunkTotalSize = 0;
		chunkReceivedSize = 0;
		chunkCounter = 0;
		data = recvBuffer;
		return true;

	} else {
		// chunk not fully read yet
		chunkReceiving = true;
		chunkTotalSize = reqSize;
		++chunkCounter;
		const int MAX_CHUNK_COUNTER = 300;
		if( chunkCounter >= MAX_CHUNK_COUNTER )
			throw ENetException( "Failed to receive expected network message" );

		if( wait )
			goto _spinwait;

		return false;
	}
}


void net::send( const void* data, int size )
{
	assert( INVALID_SOCKET != commSocket );
	int res = ::send( commSocket, (const char*)data, size, 0 );
	if( res <= 0 ) {
		NETCONS << "send failed: " << res << endl;
	} else if( res != size ) {
		NETCONS << "sent only " << res << " bytes instead of " << size << endl;
	}
}
