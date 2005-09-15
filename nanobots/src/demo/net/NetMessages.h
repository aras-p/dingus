#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

namespace net {

	enum eMessage {
		NMSG_ERROR = 0,
		NMSG_REQ_GAME_DESC = 20,
		NMSG_REQ_JOIN = 21,
		NMSG_REQ_START = 22,
		NMSG_REQ_STOP = 23,
		NMSG_REQ_SHUTDOWN = 24,
		NMSG_GAME_DESC = 50,
	};

	void	testConn();
	void	reqGameDesc();
};


#endif
