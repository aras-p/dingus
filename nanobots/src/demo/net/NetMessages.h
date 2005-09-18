#ifndef __NET_MESSAGES_H
#define __NET_MESSAGES_H

class CGameDesc;

namespace net {

	class CGameDescReader {
	public:
		enum eState { NONE, SENT_CONNTEST, GOT_CONNTEST, SENT_GAMEDESC, READ_GAMEDESC, GOT_GAMEDESC };
		
	public:
		CGameDescReader();

		void	update();

		bool	isFinished() const { return mState == GOT_GAMEDESC; }
		const std::string& getErrorMsg() const { return mErrorMsg; }
		CGameDesc* getGameDesc() { return mGameDesc; }

	private:
		eState		mState;
		std::string	mErrorMsg;
		CGameDesc*	mGameDesc;
	};

};


#endif
