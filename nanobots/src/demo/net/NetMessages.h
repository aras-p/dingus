#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

class CGameDesc;

namespace net {

	CGameDesc* receiveGameDesc( std::string& errMsg );

};


#endif
