#include "stdafx.h"
#include "GameState.h"
#include "../ByteUtils.h"
//#include "GameColors.h"


CGameState::CGameState()
{
}

CGameState::~CGameState()
{
}

void CGameState::updateState( const BYTE* data )
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
	//for each Goal
		byte	PlayerID
		byte	Mission index
		byte	Goal index
		bool	GoalReached (can be unreached)
	bool	SetCameraPosition
	//if set camera position
		byte	X
		byte	Y
		byte	Z
		byte	ToX
		byte	ToY
		byte	ToZ
	*/

	int i;

	// skip ticks
	data += 8;

	int turn = bu::readShort( data );

	// read players
	int playerCount = bu::readByte( data ); // incl. computer
	for( i = 0; i < playerCount; ++i ) {
		int pid = bu::readByte( data );
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int score = bu::readShort( data );
		std::string logtxt = bu::readStr( data );
	}

	// read bots
	int botCount = bu::readShort( data );
	for( i = 0; i < botCount; ++i ) {
		int bid = bu::readShort( data );
		int pid = bu::readByte( data );
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int type = bu::readByte( data );
		assert( type >= ENTITY_NEEDLE && type < ENTITYCOUNT );
		CGameEntity::SState state;
		state.posx = bu::readByte( data );
		state.posy = bu::readByte( data );
		state.targx = bu::readByte( data );
		state.targy = bu::readByte( data );
		state.stock = bu::readShort( data );
		state.state = bu::readByte( data );
		assert( state.state >= ENTSTATE_IDLE && state.state < ENTSTATE_DEAD );
		state.health = bu::readByte( data );
	}

	// read goals reached
	int goalCount = bu::readByte( data );
	for( i = 0; i < goalCount; ++i ) {
		int pid = bu::readByte( data );
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int mission = bu::readByte( data );
		int goal = bu::readByte( data );
		int reached = bu::readByte( data );
	}
	
	// TBD: set camera position
}
