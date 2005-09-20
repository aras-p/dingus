#include "stdafx.h"
#include "GameState.h"
#include "../ByteUtils.h"
#include "../net/NetMessages.h"
#include "../GameInfo.h"
#include "GameDesc.h"


CGameState::CGameState()
{
}

CGameState::~CGameState()
{
}

void CGameState::updateState()
{
	/*
	Format of game state data:
	Int64	TimeTicks
	Int16	Turn
	byte	NumPlayers (Pierre Team inside so 1,2 or 3)
	//for each player
		byte	PlayerID
		Int16	Score  
		string	Log text
	UInt16	Number of bots
	//for each bot
		UInt16	ID (unique ID for each bot)
		byte	PlayerID
		byte	TypeID*
		byte	X
		byte	Y
		byte	XInfo
		byte	YInfo
		UInt16	Stock
		byte	State
		byte	HitPoint
	byte	Number of Goals reached in this turn
	// for each human player (not incl. AI)
		// for each mission
			byte	MissionState
			byte	Value (0 to 100: 100==success)
	byte	SetCameraPosition
	//if set camera position
		byte	X
		byte	Y
		byte	Z
		byte	ToX
		byte	ToY
		byte	ToZ
	*/

	int i;
	const BYTE* data;

	// skip ticks
	net::receiveChunk( data, 8, true );

	int turn = bu::receiveShort();

	// read players
	int playerCount = bu::receiveByte(); // incl. AI
	for( i = 0; i < playerCount; ++i ) {
		int pid = bu::receiveByte();
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int score = bu::receiveShort();
		std::string logtxt = bu::receiveStr();
	}

	// read bots
	int botCount = bu::receiveShort();
	for( i = 0; i < botCount; ++i ) {
		int bid = bu::receiveShort();
		int pid = bu::receiveByte();
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int type = bu::receiveByte();
		assert( type >= ENTITY_NEEDLE && type < ENTITYCOUNT );
		CGameEntity::SState state;
		state.posx = bu::receiveByte();
		state.posy = bu::receiveByte();
		state.targx = bu::receiveByte();
		state.targy = bu::receiveByte();
		state.stock = bu::receiveShort();
		state.state = bu::receiveByte();
		assert( state.state >= ENTSTATE_IDLE && state.state < ENTSTATE_DEAD );
		state.health = bu::receiveByte();
	}

	// read mission states
	for( i = 1; i < playerCount; ++i ) { // human players only
		int missionCount = bu::receiveByte();
		for( int j = 0; j < missionCount; ++j ) {
			int missionState = bu::receiveByte();
			//assert( missionState >= MST_TOBEDONE && missionState < MSTCOUNT );
			int completion = bu::receiveByte();
			//assert( completion >= 0 && completion <= 100 );
		}
	}
	
	// TBD: set camera position
	int camPos = bu::receiveByte();
	if( camPos ) {
		// TBD
		net::receiveChunk( data, 6, true );
	}
}
