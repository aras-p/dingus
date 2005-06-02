#ifndef __GAME_REPLAY_H
#define __GAME_REPLAY_H

#include "ReplayEntity.h"
#include <dingus/math/Vector4.h>


class CGameReplay : public boost::noncopyable {
public:
	struct SPlayer {
		std::string	guid;
		std::string name;
		std::string county;
		int			finalScore;
		bool		winner;

		int			entityAI;	// AI entity index
	};

public:
	CGameReplay();
	~CGameReplay();

	const std::string& getGameMapName() const { return mMapName; }
	int		getGameRound() const { return mRound; }
	int		getGameTurnCount() const { return mTurnCount; }

	int		getEntityCount() const { return mEntityCount; }
	const CReplayEntity& getEntity( int index ) const { return *mEntities[index]; }

	bool	isSinglePlayer() const { return mPlayerCount==2; }
	bool	isComputerPlayer( int index ) const { return index == mPlayerCount-1; }
	int		getPlayerCount() const { return mPlayerCount; }
	const SPlayer& getPlayer( int i ) const { return mPlayers[i]; }

	void	setInfo( const std::string mapName, int round, int turnCount );
	void	setPlayer( int index, const std::string& guid, const std::string& name, const std::string& country, int finalScore, bool winner );
	void	setPlayerAIEntity( int index, int aiIndex ) { mPlayers[index].entityAI = aiIndex; }
	void	setTotalEntityCount( int ecount );
	void	setEntity( int index, CReplayEntity* e );
	void	endPlayers();

private:
	std::string	mMapName;
	int			mRound;
	int			mTurnCount;
	int			mPlayerCount; // players (incl. AI) count - 1..MAX_PLAYER_COUNT
	SPlayer		mPlayers[G_MAX_PLAYER_COUNT];

	int				mEntityCount;
	CReplayEntity**	mEntities;

public:
	int		mPropScanRadius;	// bot scan radius
	int		mPropAttackRadius;	// attack effect radius
	int		mPropMaxPlayers;	// max. human players
	int		mPropScanInterval;	// scan interval in turns
	int		mPropTurnsPerSec;	// turns per second
	int		mPropMaxBots;		// max. bots for one player at the same time
	int		mPropMaxWC;			// max. cells for computer at the same time
	int		mPropPartyLength;	// game length in minutes
	int		mPropEventInterval;	// event interval in turns
	std::string mPropDefender;	// defender assembly name
	int		mPropScoreEmptyNeedle;		// score for each on-hoshimi empty needle
	int		mPropScoreNonEmptyNeedle;	// score for each on-hoshimi non-empty needle
	int		mPropScoreAZN;		// score for each azn in the needle
};


#endif
