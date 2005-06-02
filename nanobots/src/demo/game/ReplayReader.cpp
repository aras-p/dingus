#include "stdafx.h"
#include "ReplayReader.h"
#include "GameReplay.h"
#include "../ZipReader.h"


// --------------------------------------------------------------------------

enum eReplayEntityType {
	RENTITY_NEEDLE = 0,
	RENTITY_EXPLORER = 1,
	RENTITY_COLLECTOR = 2,
	RENTITY_AI = 3,
	//RENTITY_PERFORER = 4, // not used
	RENTITY_PILOT = 5,
	RENTITY_BLOCKER = 6,
	RENTITY_CUREBOT = 7,
	RENTITY_CELL_W = 101, // temporarily swap for VS2005 special build
	RENTITY_CELL_B = 100,
	//RENTITY_CELL_W = 100,
	//RENTITY_CELL_B = 101,
};


#define READ_BYTE	*(unsigned char*)data; data += 1
#define READ_WORD	*(short*)data; data += 2



struct STmpReplayEntity : public boost::noncopyable {
	STmpReplayEntity( unsigned char etype, int eowner, int turn )
		: owner(eowner), bornTurn( turn ), deathTurn(-1)
	{
		// convert type
		switch( etype ) {
		case RENTITY_NEEDLE:	type = ENTITY_NEEDLE; break;
		case RENTITY_EXPLORER:	type = ENTITY_EXPLORER; break;
		case RENTITY_COLLECTOR:	type = ENTITY_COLLECTOR; break;
		case RENTITY_AI:		type = ENTITY_AI; break;
		//case RENTITY_PERFORER:	type = ENTITY_PERFORER; break;
		case RENTITY_PILOT:		type = ENTITY_PILOT; break;
		case RENTITY_BLOCKER:	type = ENTITY_BLOCKER; break;
		case RENTITY_CUREBOT:	type = ENTITY_CUREBOT; break;
		case RENTITY_CELL_W:	type = ENTITY_CELL_W; break;
		case RENTITY_CELL_B:	type = ENTITY_CELL_B; break;
		default: assert( false );
		}
	}


	std::vector<CReplayEntity::SState>	states;
	int				owner;
	int				bornTurn;
	int				deathTurn;
	eEntityType		type;
};


static void gReadReplayMemory( const char* data, int size, CGameReplay& game )
{
	std::vector<STmpReplayEntity*>	entities;
	fastvector<STmpReplayEntity*>	aliveEntities;

	int i;

	//
	// parse the replay

	int parseTurn = -1; // current turn #
	
	// while not read everything - read it!
	const char* dataEnd = data + size;
	while( data < dataEnd ) {

		//
		// add same state to all alive entities

		int n = aliveEntities.size();
		for( i = 0; i < n; ++i ) {
			STmpReplayEntity* e = aliveEntities[i];
			assert( !e->states.empty() );
			CReplayEntity::SState estate = e->states.back();
			e->states.push_back( estate );
		}

		//
		// parse the turn

		++parseTurn;

		// number of entities changed in this turn
		short turnEntityCount = READ_WORD;
		// read entities
		for( i = 0; i < turnEntityCount; ++i ) {

			// read entity ID
			short turnEID = READ_WORD;
			STmpReplayEntity* e;

			if( turnEID >= aliveEntities.size() ) {
				// this is a newly created entity

				assert( turnEID == aliveEntities.size() );
				// read type
				unsigned char ertype = READ_BYTE;
				bool isAI = (ertype == RENTITY_AI);
				// read owner
				int eowner = READ_BYTE;
				eowner -= 1;
				if( eowner < 0 ) {
					eowner = game.getPlayerCount()-1;
					if( ertype!=RENTITY_CELL_W && ertype!=RENTITY_CELL_B ) {
						// if we're computer, then we create w/b cells
						//ertype = RENTITY_CELL_W;
						ertype = RENTITY_CELL_B; // TODO: for VS2005 special build, black cells default
					}
				}
				if( isAI ) {
					game.setPlayerAIEntity( eowner, entities.size() );
				}
				if( ertype == RENTITY_CUREBOT ) {
					// if we're curebot, assign it to "computer" player
					eowner = game.getPlayerCount()-1;
				}
				// create and add entity
				e = new STmpReplayEntity( ertype, eowner, parseTurn );
				entities.push_back( e );
				aliveEntities.push_back( e );
				// add initial zero sate
				CReplayEntity::SState estate;
				memset( &estate, 0, sizeof(estate) );
				e->states.push_back( estate );
			} else {
				// this is an existing entity
				assert( turnEID < aliveEntities.size() );
				e = aliveEntities[turnEID];
			}

			// read number of properties
			unsigned char turnPropCount = READ_BYTE;

			// is the entity destroyed this turn?
			bool turnDeath = false;
			if( turnPropCount == 0 ) {
				// mark death turn, remove from alive entities
				e->deathTurn = parseTurn;
				aliveEntities.remove( e );
				turnDeath = true;
			}

			// fetch the properties of the last turn
			assert( !e->states.empty() );
			CReplayEntity::SState& estate = e->states.back();
			// clear actions/whatever on death turn
			if( turnDeath )
				estate.state = ENTSTATE_IDLE;

			// read changed properties
			for( int j = 0; j < turnPropCount; ++j ) {
				unsigned char propID = READ_BYTE;
				short propVal = READ_WORD;
				switch( propID ) {
				case 0:	estate.state = propVal;
					assert( estate.state >= 0 && estate.state < ENTSTATECOUNT );
					break;
				case 1: estate.posx = propVal;
					assert( estate.posx >= 0 && estate.posx <= 400 ); // TBD
					break;
				case 2: estate.posy = propVal;
					assert( estate.posy >= 0 && estate.posy <= 400 ); // TBD
					break;
				case 3: estate.targx = propVal;
					break;
				case 4: estate.targy = propVal;
					break;
				case 5: estate.azn = propVal;
					assert( estate.azn >= 0 );
					break;
				case 6: estate.health = propVal;
					assert( estate.health <= e->states[0].health );
					break;
				default: assert( false );
				}
			}

			// Workaround for broken design :)
			// at the very first turns, AI positions can be (0,0). This isn't
			// valid position for any entity, so if this turn has "correct"
			// positions - trace back and substitute those.
			if( estate.posx != 0 && estate.posy != 0 ) {
				int idx = e->states.size()-2;
				while( idx >= 0 && e->states[idx].posx == 0 && e->states[idx].posy == 0 ) {
					e->states[idx].posx = estate.posx;
					e->states[idx].posy = estate.posy;
					--idx;
				}
			}
		}
	}

	assert( data == dataEnd );
	assert( parseTurn+1 == game.getGameTurnCount() );

	//
	// now, create entities

	int n = entities.size();
	game.setTotalEntityCount( n );
	for( i = 0; i < n; ++i ) {
		STmpReplayEntity& re = *entities[i];
		// if entity is alive at game end - make it die at the one-after-last turn.
		if( re.deathTurn < 0 ) {
			re.deathTurn = game.getGameTurnCount();
			CReplayEntity::SState stlast = re.states.back();
			stlast.state = ENTSTATE_IDLE;
			re.states.push_back( stlast );
		}
		assert( re.states.size() == re.deathTurn - re.bornTurn + 1 );
		assert( re.owner >= 0 && re.owner < game.getPlayerCount() );
		CReplayEntity* e = new CReplayEntity( re.type, re.owner, re.bornTurn, re.deathTurn, &re.states[0] );
		game.setEntity( i, e );

		delete &re;
	}
}


// --------------------------------------------------------------------------


bool gReadReplay( const char* fileName, CGameReplay& game )
{
	// open zip file
	unzFile zipFile = unzOpen( fileName );
	if( zipFile == NULL ) {
		assert( false );
		return false;
	}

	//
	// read game info

	int infoSize;
	char* infoFile = gReadFileInZip( zipFile, "info", infoSize );
	if( !infoFile ) {
		unzClose( zipFile );
		assert( false );
		return false;
	}
	// game info
	const char* tokens = "\n\r";
	char* infoBuf = (char*)gSkipUTFStart( infoFile, infoSize );
	const char* infoFormat = strtok( infoBuf, tokens );
	const char* infoName = strtok( NULL, tokens );
	const char* infoRound = strtok( NULL, tokens );
	const char* infoHash = strtok( NULL, tokens );
	const char* infoTurns = strtok( NULL, tokens );
	game.setInfo( infoName, atoi(infoRound), atoi(infoTurns) );
	// players info
	int plidx = 0;
	do {
		const char* pguid = strtok( NULL, tokens );
		if( !strcmp(pguid,"****") )
			break;
		const char* pname = strtok( NULL, tokens );
		const char* pcountry = strtok( NULL, tokens );
		const char* pscore = strtok( NULL, tokens );
		const char* pwinner = strtok( NULL, tokens );
		game.setPlayer( plidx, pguid, pname, pcountry, atoi(pscore), !strcmp(pwinner,"True") );
		++plidx;
	} while(true);
	game.endPlayers();
	// properties
	const char* tokens2 = "=\n\r";
	do {
		const char* pprop = strtok( NULL, tokens2 );
		if( !pprop || pprop[0] == 0 )
			break;
		const char* pval = strtok( NULL, tokens );
		if( !stricmp(pprop,"ScanLength") )
			game.mPropScanRadius = atoi( pval );
		else if( !stricmp(pprop,"DefenseLength") )
			game.mPropAttackRadius = atoi( pval );
		else if( !stricmp(pprop,"MaxPlayers") )
			game.mPropMaxPlayers = atoi( pval );
		else if( !stricmp(pprop,"NumberOfTurnBetweenScan") )
			game.mPropScanInterval = atoi( pval );
		else if( !stricmp(pprop,"NumberOfTurnBySecond") )
			game.mPropTurnsPerSec = atoi( pval );
		else if( !stricmp(pprop,"NbrMaxBots") )
			game.mPropMaxBots = atoi( pval );
		else if( !stricmp(pprop,"NbrMaxWhiteCells") )
			game.mPropMaxWC = atoi( pval );
		else if( !stricmp(pprop,"PartyLength") )
			game.mPropPartyLength = atoi( pval );
		else if( !stricmp(pprop,"NumberOfTurnBetweenEvent") )
			game.mPropEventInterval = atoi( pval );
		else if( !stricmp(pprop,"NeedleScore") )
			game.mPropScoreEmptyNeedle = atoi( pval );
		else if( !stricmp(pprop,"ValidNeedleScore") )
			game.mPropScoreNonEmptyNeedle = atoi( pval );
		else if( !stricmp(pprop,"AZNScore") )
			game.mPropScoreAZN = atoi( pval );
		else if( !stricmp(pprop,"DefenderName") )
			game.mPropDefender = pval;
	} while(true);
	
	delete[] infoFile;


	//
	// read replay

	int replaySize;
	char* replayFile = gReadFileInZip( zipFile, "replay", replaySize );
	if( !replayFile ) {
		unzClose( zipFile );
		assert( false );
		return false;
	}
	gReadReplayMemory( replayFile, replaySize, game );

	delete[] replayFile;

	// close zip file
	unzClose( zipFile );
	
	return true;
}
