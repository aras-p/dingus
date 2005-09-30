#ifndef __GAME_STATE_H
#define __GAME_STATE_H

#include "GameEntity.h"
#include <dingus/utils/ringdeque.h>


struct SServerState {
	SServerState() { memset(this,0,sizeof(SServerState)); }
	
	eGameServerState state;
	bool	playerRealtime[G_MAX_PLAYERS]; // first is dummy (AI)
	bool	playerControlled[G_MAX_PLAYERS]; // first is dummy (AI)
};


class CGameState : public boost::noncopyable {
public:
	enum { MAX_LOG_MSGS = 16 };
	struct SLogMsg {
		int			turn;
		std::string	message;
	};
	struct SPlayer {
		SPlayer() : score(0), injectionPtX(0), injectionPtY(0), aiID(-1), botCount(0) { }
		int		score;
		ringdeque<SLogMsg,MAX_LOG_MSGS>	logs; // [0] is the most recent one
		int		injectionPtX;
		int		injectionPtY;
		int		aiID;
		int		botCount;				// alive entity count
	};

	typedef std::map<int,CGameEntity*>	TEntityMap;

public:
	CGameState();
	~CGameState();

	void updateState();

	int getEntityCount() const { return mEntities.size(); }
	TEntityMap::const_iterator entitiesBegin() const { return mEntities.begin(); }
	TEntityMap::const_iterator entitiesEnd() const { return mEntities.end(); }

	int getTurn() const { return mTurn; }
	const time_value& getTurnReceivedTime() const { return mTurnReceivedTime; }

	//bool	isSinglePlayer() const { return mPlayerCount==2; }
	//bool	isComputerPlayer( int index ) const { return index == mPlayerCount-1; }
	const SPlayer& getPlayer( int i ) const { return mPlayers[i]; }

	const SServerState& getServerState() const { return mServerState; }
	void	updateServerState( bool forceNow, bool sendStart );

private:
	SPlayer		mPlayers[G_MAX_PLAYERS]; // incl. AI
	TEntityMap	mEntities;
	int			mTurn;				///< Current turn
	time_value	mTurnReceivedTime;	///< When the current turn was received

	SServerState	mServerState;
	std::string		mServerStateErrMsg;
	time_value		mLastStateQueryTime;
};


#endif
