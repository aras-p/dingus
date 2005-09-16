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
#include "net/NetInterface.h"
#include "net/NetMessages.h"


std::string gErrorMsg = "";


// --------------------------------------------------------------------------
//  singleton

CGameInfo* CGameInfo::mSingleInstance = 0;

void CGameInfo::initialize( const std::string& server, int port )
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

const char* CGameInfo::initBegin()
{
	return "Connecting to server...";
	gErrorMsg = "";
}

const char* CGameInfo::initStep()
{
	gErrorMsg = "";

	// connect to server?
	if( !net::isConnected() ) {
		try {
			net::initialize( mServerName.c_str(), mServerPort );
		} catch( const net::ENetException& e ) {
			gErrorMsg = e.what();
			return NULL;
		}
		return "Requesting game desc...";
	}

	// request game desc?
	if( !mGameDesc ) {
		bool connOk = net::testConn();
		if( !connOk	) {
			gErrorMsg = "Wrong connection protocol";
			return NULL;
		}
		mGameDesc = new CGameDesc();
		std::string errmsg = net::getGameDesc( *mGameDesc );
		if( !errmsg.empty()	) {
			gErrorMsg = errmsg;
			return NULL;
		}
		return "Calculating level mesh...";
	}

	// calculate level mesh?
	if( !mLevelMesh ) {
		// TBD: error processing
		mLevelMesh = new CLevelMesh( mGameDesc->getMap() );
		return "Initializing level points mesh...";
	}
	assert( mLevelMesh );

	// calculate level points mesh?
	if( !mPointsMesh ) {
		// TBD: error processing
		mPointsMesh = new CPointsMesh( mGameDesc->getMap(), *mLevelMesh );
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


CGameInfo::CGameInfo( const std::string& server, int port )
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
