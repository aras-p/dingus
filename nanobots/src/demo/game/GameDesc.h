#ifndef __GAME_DESC_H
#define __GAME_DESC_H

#include "GameTypes.h"
#include "GameMap.h"


class CGameDesc : public boost::noncopyable {
public:
	enum { FLAG_SIZE = 64 };
	struct SPlayer {
		std::string name;
		D3DCOLOR	flag[FLAG_SIZE*FLAG_SIZE];
	};

	struct SMission {
		std::string	desc;
		std::vector< std::pair<int,int> > points;
	};

public:
	CGameDesc();
	~CGameDesc();

	/// @return Empty string if ok, error message on error.
	std::string	initialize( const BYTE* gameDescData );

	const CGameMap& getMap() const { return mMap; }
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
	CGameMap	mMap;
	int			mPlayerCount; // players count (incl AI)
	SPlayer		mPlayers[G_MAX_PLAYERS];

	std::string	mMapName;
	int			mTurnCount; // -1 if realtime play

	int			mBlockerLength;
	
	std::string				mMissionSummary;
	std::vector<SMission>	mMissions;
};


#endif
