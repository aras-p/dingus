#ifndef __GAME_DESC_H
#define __GAME_DESC_H

#include "GameTypes.h"


class CGameDesc : public boost::noncopyable {
public:
	enum { FLAG_SIZE = 64 };
	struct SPlayer {
		std::string name;
		D3DCOLOR	flag[FLAG_SIZE*FLAG_SIZE];
	};

public:
	CGameDesc();
	~CGameDesc();

	const std::string& getMapName() const { return mMapName; }
	int		getTurnCount() const { return mTurnCount; }

	//bool	isSinglePlayer() const { return mPlayerCount==2; }
	//bool	isComputerPlayer( int index ) const { return index == mPlayerCount-1; }
	int		getPlayerCount() const { return mPlayerCount; }
	const SPlayer& getPlayer( int i ) const { return mPlayers[i]; }

	int		getBlockerLength() const { return mBlockerLength; }

	//void	setInfo( const std::string mapName, int round, int turnCount );
	//void	setPlayer( int index, const std::string& guid, const std::string& name, const std::string& country, int finalScore, bool winner );
	//void	endPlayers();

private:
	int			mPlayerCount; // players count (not incl. AI)
	SPlayer		mPlayers[G_MAX_PLAYERS];

	std::string	mMapName;
	int			mTurnCount; // -1 if realtime play

	int			mBlockerLength;
};


#endif
