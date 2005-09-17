#include "stdafx.h"
#include "GameDesc.h"
#include "GameColors.h"
#include "../ByteUtils.h"


CGameDesc::CGameDesc()
:	mPlayerCount(0),
	mTurnCount(-1),
	mBlockerLength(1)
{
	assert( G_MAX_PLAYERS == 3 );

	const float CM = 0.50f;
	const float CT = 0.75f;
	
	// computer team - white
	gColors.team[0].main.set( D3DXCOLOR(1,1,1,1) );
	gColors.team[0].tone.set( D3DXCOLOR(1,1,1,1) );
	// first team - greenish
	gColors.team[1].main.set( D3DXCOLOR(CM,1,CM,1) );
	gColors.team[1].tone.set( D3DXCOLOR(CT,1,CT,1) );
	// second team - blueish
	gColors.team[2].main.set( D3DXCOLOR(CM,CM,1,1) );
	gColors.team[2].tone.set( D3DXCOLOR(CT,CT,1,1) );

	// azn - cyan
	gColors.ptAZN.main.set( 0xFF30a0a0 );
	gColors.ptAZN.tone.set( 0xFF30a0a0 );
	// hoshimi - red
	gColors.ptHoshimi.main.set( 0xFFa03030 );
	gColors.ptHoshimi.tone.set( 0xFFa03030 );
}

CGameDesc::~CGameDesc()
{
}

std::string CGameDesc::initialize( const BYTE* gameDescData )
{
	/*
	Data format:

	byte	GameServerState
	byte	NumberOfPlayers (0, 1 or 2)
	Int16	NumberOfTurn (-1 if replay)
	byte	BlockerLength
	//Player1
		string	Player1 name
		int32	Player1 Flag length
		byte[]	Player1 Flag (64*64 bitmap)
	//if NumOfPlayers == 2, Player2
		string	Player2 name
		int32	Player2 Flag length
		byte[]	Player2 Flag (64*64 bitmap)
	// Map data follows...
		...
	// Missions
	string	Briefing description
	byte	Number of missions
	//for each Mission
		string	Mission description
		byte	Number of objective points of the mission
		//for each objective point
			byte	X
			byte	Y
	*/

	int i;

	// TBD: seems that this isn't present
	//BYTE gameState = bu::readByte( gameDescData );
	//if( gameState >= GST_NONE || gameState < GST_ENDED )
	//	return "Invalid game server state";

	mPlayerCount = bu::readByte( gameDescData ) + 1;
	if( mPlayerCount < 1 || mPlayerCount > G_MAX_PLAYERS )
		return "Invalid player count";

	mTurnCount = bu::readShort( gameDescData );

	mBlockerLength = bu::readByte( gameDescData );
	if( mBlockerLength < 1 || mBlockerLength > 20 )
		return "Invalid blocker length";

	// read players, start at first player (1)
	for( i = 1; i < mPlayerCount; ++i ) {
		mPlayers[i].name = bu::readStr( gameDescData );
		// TBD: flag bitmap
		int flagSize = bu::readInt( gameDescData );
		gameDescData += flagSize;
	}

	// initialize game map
	std::string errmsg = mMap.initialize( gameDescData );
	if( !errmsg.empty() )
		return errmsg;

	// read missions
	mMissionSummary = bu::readStr( gameDescData );
	int missionCount = bu::readByte( gameDescData );
	mMissions.reserve( missionCount );
	for( i = 0; i < missionCount; ++i ) {
		mMissions.push_back( SMission() );
		SMission& m = mMissions.back();
		m.desc = bu::readStr( gameDescData );
		
		// TBD: that's weird!
		if( i == 0 )
			gameDescData++;

		int ptCount = bu::readByte( gameDescData );
		m.points.reserve( ptCount );
		for( int j = 0; j < ptCount; ++j ) {
			int ptx = bu::readByte( gameDescData );
			int pty = bu::readByte( gameDescData );
			m.points.push_back( std::make_pair(ptx,pty) );
		}
	}

	return "";
}


/*
void CGameDesc::setInfo( const std::string mapName, int round, int turnCount )
{
	mMapName = mapName;
	mRound = round;
	mTurnCount = turnCount;
}

void CGameDesc::setPlayer( int index, const std::string& guid, const std::string& name, const std::string& country, int finalScore, bool winner )
{
	assert( index == mPlayerCount );
	assert( index >= 0 && index < G_MAX_PLAYER_COUNT );

	++mPlayerCount;
	SPlayer& p = mPlayers[index];
	p.guid = guid;
	p.name = name;
	p.county = country;
	p.finalScore = finalScore;
	p.winner = winner;
	p.entityAI = 0; // will be set later
}

void CGameDesc::endPlayers()
{
}
*/
