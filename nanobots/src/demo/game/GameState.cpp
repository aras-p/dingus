#include "stdafx.h"
#include "GameState.h"
#include "../ByteUtils.h"
#include "../net/NetMessages.h"
#include "../GameInfo.h"
#include "GameDesc.h"


CGameState::CGameState()
:	mLastStateQueryTime(-1)
,	mTurn(0)
,	mFirstTurn(-1)
,	mGameEnded(false)
,	mWinner(-1)
{
	int nmissions = CGameInfo::getInstance().getGameDesc().getMissionCount();
	for( int i = 0; i < G_MAX_PLAYERS; ++i ) {
		mPlayers[i].missions.resize( nmissions );
		for( int m = 0; m < nmissions; ++m ) {
			SMissionStatus& mst = mPlayers[i].missions[m];
			mst.state = MST_TOBEDONE;
			mst.completion = 0.0f;
		}
	}
}

CGameState::~CGameState()
{
}

void CGameState::updateState( int winnerPlayer )
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
		byte	Injection Point X
		byte	Injection Point Y
	UInt16	Number of bots
	//for each bot
		UInt16	ID (unique ID for each bot)
		byte	PlayerID
		byte	TypeID*
		string	FriendlyName (8 chars max)
		byte	X
		byte	Y
		byte	XInfo
		byte	YInfo
		UInt16	Stock
		byte	State
		byte	HitPoint
	// for each human player (not incl. AI)
		byte	Number of missions
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

	if( winnerPlayer != -1 ) {
		mGameEnded = true;
		mWinner = winnerPlayer;
	}

	int i;
	const BYTE* data;

	// skip ticks
	net::receiveChunk( data, 8, true );

	int turn = bu::receiveShort();
	if( turn != mTurn ) {
		mTurnReceivedTime = CSystemTimer::getInstance().getTime();
		if( mFirstTurn < 0 )
			mFirstTurn = turn;
	}

	// read players, clear stats
	int playerCount = bu::receiveByte(); // incl. AI
	for( i = 0; i < playerCount; ++i ) {
		int pid = bu::receiveByte();
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int score = bu::receiveShort();
		std::string logtxt = bu::receiveStr();
		int injX = bu::receiveByte();
		int injY = bu::receiveByte();
		if( mTurn != turn ) { // if it's really a new turn, update players
			SPlayer& pl = mPlayers[i];
			pl.score = score;

			pl.botCount = 0;

			SLogMsg logmsg;
			logmsg.turn = turn;
			logmsg.message = logtxt;
			if( pl.logs.size() == pl.logs.capacity() )
				pl.logs.pop_back();
			pl.logs.push_front( logmsg );

			// if the injection point just appeared - create it
			if( pl.injectionPtX == 0 && pl.injectionPtY == 0 &&
				injX != 0 && injY != 0 )
			{
				CGameInfo::getInstance().onNewInjectionPoint( i, injX, injY );
			}
			pl.injectionPtX = injX;
			pl.injectionPtY = injY;
		}
	}

	// read bots, update player stats
	int botCount = bu::receiveShort();
	for( i = 0; i < botCount; ++i ) {
		int bid = bu::receiveShort();
		int pid = bu::receiveByte();
		assert( pid >= 0 && pid < G_MAX_PLAYERS );
		int type = bu::receiveByte();
		assert( type >= ENTITY_NEEDLE && type < ENTITYCOUNT );
		std::string fname = bu::receiveStr();
		if( fname.length() > CGameEntity::MAX_NAME_LEN )
			fname.resize( CGameEntity::MAX_NAME_LEN );

		CGameEntity::SState state;
		state.posx = bu::receiveByte();
		state.posy = bu::receiveByte();
		state.targx = bu::receiveByte();
		state.targy = bu::receiveByte();
		state.stock = bu::receiveShort();
		state.state = bu::receiveByte();
		assert( state.state >= ENTSTATE_IDLE && state.state < ENTSTATE_DEAD );
		state.health = bu::receiveByte();

		memcpy( state.name, fname.c_str(), fname.length() );
		state.name[fname.length()] = 0;

		if( mTurn != turn ) { // if it's really a new turn, add/update entity
			++mPlayers[pid].botCount;

			CGameEntity* entity = NULL;
			bool newEntity = false;
			TEntityMap::iterator it = mEntities.find( bid );
			if( it == mEntities.end() ) {
				entity = new CGameEntity( bid, (eEntityType)type, pid, turn );
				mEntities.insert( std::make_pair( bid, entity ) );
				newEntity = true;
			} else {
				entity = it->second;
			}
			entity->updateState( turn, state );

			if( newEntity ) {
				CGameInfo::getInstance().onNewEntity( *entity );
				// detect born AI
				if( type == ENTITY_AI ) {
					mPlayers[pid].aiID = bid;
				}
			}
		}
	}

	// read mission states
	for( i = 1; i < playerCount; ++i ) { // human players only
		int missionCount = bu::receiveByte();
		for( int j = 0; j < missionCount; ++j ) {
			int missionState = bu::receiveByte();
			assert( missionState >= MST_TOBEDONE && missionState < MSTCOUNT );
			int completion = bu::receiveByte();
			assert( completion >= 0 && completion <= 100 );

			mPlayers[i].missions[j].state = (eMissionState)missionState;
			mPlayers[i].missions[j].completion = completion / 100.0f;
		}
	}
	
	// TBD: set camera position
	int camPos = bu::receiveByte();
	if( camPos ) {
		// TBD
		net::receiveChunk( data, 6, true );
	}

	// now that all entities are updated, go through existing ones and
	// mark the ones that were not present now as dead
	TEntityMap::iterator it, itEnd = mEntities.end();
	for( it = mEntities.begin(); it != itEnd; ++it ) {
		CGameEntity& e = *it->second;
		if( e.getLastUpdateTurn() < turn ) {
			e.markDead( turn );
			// detect died AIs
			if( e.getType() == ENTITY_AI ) {
				mPlayers[e.getOwner()].aiID = -1;
			}
		}
	}

	mTurn = turn;
}


void CGameState::updateServerState( bool forceNow, bool sendStart )
{
	time_value currT = CSystemTimer::getInstance().getTime();
	if( forceNow || currT - mLastStateQueryTime > time_value::fromsec(0.5f) ) {
		const CGameDesc& desc = CGameInfo::getInstance().getGameDesc();
		mLastStateQueryTime = currT;
		net::receiveServerState( desc.getPlayerCount(), mServerState, mServerStateErrMsg, sendStart );
	}
}
