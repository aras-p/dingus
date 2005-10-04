#include "stdafx.h"
#include "GameDesc.h"
#include "GameColors.h"
#include "../ByteUtils.h"
#include "../DemoResources.h"
#include <dingus/utils/StringHelper.h>


CGameDesc::CGameDesc()
:	mPlayerCount(0),
	mTurnCount(-1),
	mTurnsPerSecond(5),
	mTurnDT(2),
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
	// objective - yellow
	gColors.ptObjective.main.set( 0xFFf0f030 );
	gColors.ptObjective.tone.set( 0xFFa0a030 );
}

CGameDesc::~CGameDesc()
{
}


// --------------------------------------------------------------------------


class CPlayerFlagTextureCreator : public CFixedTextureCreator {
public:
	CPlayerFlagTextureCreator( const CGameDesc::SPlayer& player )
	:	CFixedTextureCreator( CGameDesc::FLAG_SIZE, CGameDesc::FLAG_SIZE, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED )
	,	mPlayer( &player )
	{
	}

	virtual IDirect3DTexture9* createTexture() {
		IDirect3DTexture9* tex = CFixedTextureCreator::createTexture();
		HRESULT hr;

		IDirect3DSurface9* dstSurf = 0;
		hr = tex->GetSurfaceLevel( 0, &dstSurf );
		assert( SUCCEEDED(hr) );
		hr = D3DXLoadSurfaceFromFileInMemory( dstSurf, NULL, NULL, &mPlayer->flagBMP[0], mPlayer->flagBMP.size(), NULL, D3DX_DEFAULT, 0, NULL );
		if( FAILED(hr) ) {
			// silently fill surface with color
			// TBD
			CONS << "WARN: Player " << mPlayer->name << " has incorrect flag bitmap" << endl;
		}

		dstSurf->Release();

		D3DXFilterTexture( tex, NULL, 0, D3DX_FILTER_BOX );
		return tex;
	}

private:
	const CGameDesc::SPlayer*	mPlayer;
};


// --------------------------------------------------------------------------


/*
Data format:

byte	GameServerState
byte	NumberOfPlayers (0, 1 or 2)
Int16	NumberOfTurn (-1 if replay)
byte	BlockerLength
byte	TurnsPerSecond
byte	[reserved]
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
	byte	Mission type
	string	Mission description
	byte	Number of objective points of the mission
	//for each objective point
		byte	X
		byte	Y
*/

std::string CGameDesc::initialize()
{
	int i;

	const BYTE* data;
	net::receiveChunk( data, 6, true ); // TBD: 7

	// TBD: seems that this isn't present
	//BYTE gameState = bu::readByte( gameDescData );
	//if( gameState >= GST_NONE || gameState < GST_READYTOSTART )
	//	return "Invalid game server state";

	mPlayerCount = bu::readByte( data ) + 1;
	if( mPlayerCount < 2 || mPlayerCount > G_MAX_PLAYERS )
		return "Invalid player count";

	mTurnCount = bu::readShort( data );

	mBlockerLength = bu::readByte( data );
	if( mBlockerLength < 1 || mBlockerLength > 20 )
		return "Invalid blocker length";

	mTurnsPerSecond = bu::readByte( data );
	if( mTurnsPerSecond < 1 || mTurnsPerSecond > 50 )
		return "Invalid turns/second value";
	mTurnDT = 1.0f / mTurnsPerSecond;

	// read players, start at first player (1)
	const char* PLAYER_TEX_NAMES[G_MAX_PLAYERS] = {
		NULL,
		RID_TEX_PLAYER1,
		RID_TEX_PLAYER2,
	};
	for( i = 1; i < mPlayerCount; ++i ) {
		mPlayers[i].name = bu::receiveStr();
		// TBD: workaround around Richard's funky stuff
		int lastSlash = mPlayers[i].name.find_last_of( "\\//" );
		if( lastSlash >= 0 )
			mPlayers[i].name = mPlayers[i].name.substr( lastSlash+1, mPlayers[i].name.length()-lastSlash );

		// flag bitmap
		int flagSize = bu::receiveInt();
		mPlayers[i].flagBMP.resize( flagSize );
		net::receiveChunk( data, flagSize, true );
		memcpy( &mPlayers[i].flagBMP[0], data, flagSize );

		// register level texture
		CSharedTextureBundle::getInstance().registerTexture( PLAYER_TEX_NAMES[i], *new CPlayerFlagTextureCreator( mPlayers[i] ) );
	}

	// initialize game map
	std::string errmsg = mMap.initialize();
	if( !errmsg.empty() )
		return errmsg;

	// read missions
	mMissionSummary = bu::receiveStr();
	CStringHelper::trimString( mMissionSummary );
	mMissionSummaryLines = CStringHelper::countLines( mMissionSummary );

	int missionCount = bu::receiveByte();
	mMissions.reserve( missionCount );
	for( i = 0; i < missionCount; ++i ) {
		mMissions.push_back( SMission() );
		SMission& m = mMissions.back();
		bu::receiveByte(); // TBD: assign mission type
		m.desc = bu::receiveStr();
		CStringHelper::trimString( m.desc );
		m.descLines = CStringHelper::countLines( m.desc );
		
		int ptCount = bu::receiveByte();
		m.points.reserve( ptCount );
		for( int j = 0; j < ptCount; ++j ) {
			int ptx = bu::receiveByte();
			int pty = bu::receiveByte();
			m.points.push_back( std::make_pair(ptx,pty) );
			mMap.addObjectivePoint( i, ptx, pty );
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
