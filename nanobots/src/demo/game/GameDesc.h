#ifndef __GAME_DESC_H
#define __GAME_DESC_H

#include "GameTypes.h"
#include "GameMap.h"


class CGameDesc : public boost::noncopyable {
public:
	enum { FLAG_SIZE = 64 };
	struct SPlayer {
		std::string			name;
		std::vector<BYTE>	flagBMP;
	};

	struct SMission {
		std::string	desc;
		int			descLines;
		std::vector< std::pair<int,int> > points;
	};

public:
	CGameDesc();
	~CGameDesc();


	/// @return Error message or empty string
	std::string	initialize();


	const CGameMap& getMap() const { return mMap; }
	CGameMap& getMap() { return mMap; }
	int		getTurnCount() const { return mTurnCount; }
	int		getTurnsPerSecond() const { return mTurnsPerSecond; }
	float	getTurnDT() const { return mTurnDT; }

	//bool	isSinglePlayer() const { return mPlayerCount==2; }
	//bool	isComputerPlayer( int index ) const { return index == mPlayerCount-1; }
	int		getPlayerCount() const { return mPlayerCount; }
	const SPlayer& getPlayer( int i ) const { return mPlayers[i]; }

	int		getBlockerLength() const { return mBlockerLength; }

	const std::string& getMissionSummary() const { return mMissionSummary; }
	int		getMissionSummaryLines() const { return mMissionSummaryLines; }
	int		getMissionCount() const { return mMissions.size(); }
	const SMission& getMission( int idx ) const { return mMissions[idx]; }

private:
	CGameMap	mMap;
	int			mPlayerCount; // players count (incl AI)
	SPlayer		mPlayers[G_MAX_PLAYERS];

	int			mTurnCount; // -1 if realtime play
	int			mTurnsPerSecond;
	float		mTurnDT; // second / mTurnsPerSecond

	int			mBlockerLength;
	
	std::string		mMissionSummary;
	int				mMissionSummaryLines;
	std::vector<SMission>	mMissions;
};


#endif
