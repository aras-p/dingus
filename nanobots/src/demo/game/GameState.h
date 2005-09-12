#ifndef __GAME_STATE_H
#define __GAME_STATE_H

#include "GameEntity.h"
#include <dingus/utils/ringdeque.h>


class CGameState : public boost::noncopyable {
public:
	enum { MAX_LOG_MSGS = 16 };
	struct SLogMsg {
		int			turn;
		std::string	message;
	};
	struct SPlayer {
		int		score;
		ringdeque<SLogMsg,MAX_LOG_MSGS>	logs;
		int		aiID;
	};

public:
	CGameState();
	~CGameState();

	//int		getEntityCount() const { return mEntityCount; }
	//const CReplayEntity& getEntity( int index ) const { return *mEntities[index]; }

	//bool	isSinglePlayer() const { return mPlayerCount==2; }
	//bool	isComputerPlayer( int index ) const { return index == mPlayerCount-1; }
	//int		getPlayerCount() const { return mPlayerCount; }
	//const SPlayer& getPlayer( int i ) const { return mPlayers[i]; }

private:
	SPlayer		mPlayers[G_MAX_PLAYERS];
};


#endif
