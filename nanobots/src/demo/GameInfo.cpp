#include "stdafx.h"
#include "GameInfo.h"
//#include "map/GameMap.h"
#include "map/LevelMesh.h"
#include "map/PointsMesh.h"
#include "game/GameDesc.h"
#include "game/GameState.h"
#include "entity/EntityManager.h"
#include "MinimapRenderer.h"
#include "EntityInfoRenderer.h"


std::string gErrorMsg = "";


// --------------------------------------------------------------------------
//  singleton

CGameInfo* CGameInfo::mSingleInstance = 0;

void CGameInfo::initialize( const char* server, int port )
{
	assert( !mSingleInstance );
	mSingleInstance = new CGameInfo( server, port );
}

void CGameInfo::finalize()
{
	assert( mSingleInstance );
	delete mSingleInstance;
}

// --------------------------------------------------------------------------
//  multi-step initialization

/*
const char* CGameInfo::initBegin()
{
	return "Loading replay...";
	gErrorMsg = "";
}

const char* CGameInfo::initStep()
{
	gErrorMsg = "";

	// load replay?
	if( !mReplay ) {
		mReplay = new CGameReplay();
		bool ok = gReadReplay( ("replays/" + mReplayFile).c_str(), *mReplay );
		if( !ok ) {
			gErrorMsg = "Error loading replay '" + mReplayFile + "'";
			return NULL;
		}
		return "Loading game map...";
	}
	assert( mReplay );

	// load game map?
	if( !mGameMap ) {
		mGameMap = new CGameMap();
		std::string errMsg = mGameMap->initialize( "tissues/" + mReplay->getGameMapName(), mReplay->getGameMapName() );
		if( !errMsg.empty() ) {
			gErrorMsg = "Error loading game map: " + errMsg;
			return NULL;
		}
		return "Calculating level mesh...";
	}
	assert( mGameMap );

	// calculate level mesh?
	if( !mLevelMesh ) {
		// TBD: error processing
		mLevelMesh = new CLevelMesh( *mGameMap );
		return "Initializing level points mesh...";
	}
	assert( mLevelMesh );

	// calculate level points mesh?
	if( !mPointsMesh ) {
		// TBD: error processing
		mPointsMesh = new CPointsMesh( *mGameMap, *mLevelMesh );
		return "Creating renderers...";
	}
	assert( mPointsMesh );

	// create renderers?
	if( !mMinimapRenderer ) {
		mMinimapRenderer = new CMinimapRenderer( *RGET_IB(RID_IB_QUADS) );
		mEntityBlobsRenderer = new CMinimapRenderer( *RGET_IB(RID_IB_QUADS) );
		mEntityInfoRenderer = new CEntityInfoRenderer( *RGET_IB(RID_IB_QUADS) );
		return "Creating entities...";
	}

	// create entity manager?
	if( !mEntities ) {
		mEntities = new CEntityManager();
		return NULL; // all done!
	}

	assert( false ); // shouldn't get here...
	return NULL;
}
*/


CGameInfo::CGameInfo( const char* server, int port )
:	mServerName(server)
,	mServerPort(port)
//:	mTime( 0.0f ),
//	mReplay(NULL),
//	mGameMap(NULL),
,	mGameDesc(NULL)
,	mState(NULL)
,	mLevelMesh(NULL),
	mPointsMesh(NULL),
	mMinimapRenderer(NULL), mEntityBlobsRenderer(NULL), mEntityInfoRenderer(NULL),
	mEntities(NULL)
{
}


CGameInfo::~CGameInfo()
{
	safeDelete( mEntities );
	
	safeDelete( mMinimapRenderer );
	safeDelete( mEntityBlobsRenderer );
	safeDelete( mEntityInfoRenderer );
	
	safeDelete( mPointsMesh );
	safeDelete( mLevelMesh );
	safeDelete( mState );
	safeDelete( mGameDesc );
}
