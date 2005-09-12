#include "stdafx.h"
#include "GameState.h"
#include "GameColors.h"


CGameState::CGameState()
:	mPlayerCount(0),
	mEntityCount(0),
	mEntities(NULL),
	mPropScanRadius(10), mPropAttackRadius(4),
	mPropMaxPlayers(2), mPropScanInterval(6),
	mPropTurnsPerSec(6), mPropMaxBots(40),
	mPropMaxWC(40), mPropPartyLength(1),
	mPropEventInterval(3), mPropScoreEmptyNeedle(5), mPropScoreNonEmptyNeedle(20), mPropScoreAZN(1)
{
}

CGameState::~CGameState()
{
	if( mEntities ) {
		for( int i = 0; i < mEntityCount; ++i )
			delete mEntities[i];
		delete[] mEntities;
	}
}

void CGameState::setInfo( const std::string mapName, int round, int turnCount )
{
	mMapName = mapName;
	mRound = round;
	mTurnCount = turnCount;
}

void CGameState::setPlayer( int index, const std::string& guid, const std::string& name, const std::string& country, int finalScore, bool winner )
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

void CGameState::endPlayers()
{
	assert( G_MAX_PLAYER_COUNT == 3 );
	const float CM = 0.50f;
	const float CT = 0.75f;
	// first team - greenish
	gColors.team[0].main.set( D3DXCOLOR(CM,1,CM,1) );
	gColors.team[0].tone.set( D3DXCOLOR(CT,1,CT,1) );
	// second team - blueish
	gColors.team[1].main.set( D3DXCOLOR(CM,CM,1,1) );
	gColors.team[1].tone.set( D3DXCOLOR(CT,CT,1,1) );
	// computer team - white
	gColors.team[mPlayerCount-1].main.set( D3DXCOLOR(1,1,1,1) );
	gColors.team[mPlayerCount-1].tone.set( D3DXCOLOR(1,1,1,1) );

	// azn - cyan
	gColors.ptAZN.main.set( 0xFF30a0a0 );
	gColors.ptAZN.tone.set( 0xFF30a0a0 );
	// hoshimi - red
	gColors.ptHoshimi.main.set( 0xFFa03030 );
	gColors.ptHoshimi.tone.set( 0xFFa03030 );
}

void CGameState::setTotalEntityCount( int ecount )
{
	assert( !mEntities );
	assert( ecount > 0 );
	mEntityCount = ecount;
	mEntities = new CReplayEntity*[ ecount ];
	memset( mEntities, 0, ecount*sizeof(CReplayEntity*) );
}

void CGameState::setEntity( int index, CReplayEntity* e )
{
	assert( index >= 0 && index < mEntityCount );
	assert( e );
	assert( !mEntities[index] );
	mEntities[index] = e;
}
