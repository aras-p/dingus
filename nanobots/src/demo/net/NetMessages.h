#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

class CGameDesc;

namespace net {

	bool	testConn();
	std::string	getGameDesc( CGameDesc& desc );
};


#endif
