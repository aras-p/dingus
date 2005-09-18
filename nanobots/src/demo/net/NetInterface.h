#ifndef __NET_IFACE_H
#define __NET_IFACE_H


namespace net {
	extern CConsoleChannel& NETCONS;
	const int NET_ASYNC_MESSAGE = WM_USER+21;

	struct ENetException : public std::runtime_error {
		ENetException( const std::string& msg ) : runtime_error(msg) { }
	};

	void	initialize( const char* serverName, int serverPort, HWND wnd );
	void	shutdown();
	bool	isConnected();

	void	onAsyncMsg( WPARAM wparam, LPARAM lparam );

	/**
	 *	An application requests received data of reqSize size.
	 *  While data is not available or not fully available, this will
	 *  return false. Just keep calling it each frame (i.e. not in a loop)
	 *  until you receive true.
	 */
	bool	receiveChunk( const unsigned char*& data, int reqSize, bool wait = false );

	void	send( const void* data, int size );
};


#endif
