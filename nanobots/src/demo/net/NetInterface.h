#ifndef __NET_IFACE_H
#define __NET_IFACE_H


namespace net {
	extern CConsoleChannel& NETCONS;

	struct ENetException : public std::runtime_error {
		ENetException( const std::string& msg ) : runtime_error(msg) { }
	};

	void	initialize( const char* serverName, int serverPort );
	void	shutdown();
	bool	isConnected();
	
	bool	receive( const unsigned char*& data, int& size );
	void	send( const void* data, int size );
};


#endif
