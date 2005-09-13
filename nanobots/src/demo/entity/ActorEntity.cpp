#include "stdafx.h"
#include "ActorEntity.h"
#include "../game/GameEntity.h"
#include "../game/GameState.h"
#include "../game/GameDesc.h"
#include "../game/GameColors.h"
#include "../GameInfo.h"
#include "../map/LevelMesh.h"
#include "../DemoResources.h"
#include <dingus/utils/Random.h>


CActorEntity::CActorEntity( const CGameEntity& ge )
:	CMeshEntity( ge.getTypeName(), LOD_COUNT ),
	mSoundPlayedTurn(-10),
	mGameEntity( &ge ),
	mOutlineTTL(0.0f)
{
	const CGameInfo& gi = CGameInfo::getInstance();
	const CLevelMesh& levelMesh = gi.getLevelMesh();
	const CGameMap& gmap = gi.getGameDesc().getMap();

	eEntityType etype = ge.getType();

	// set up colors
	bool ai = (etype == ENTITY_AI);
	mColorMinimap = ai ? gColors.team[ge.getOwner()].main.c : gColors.team[ge.getOwner()].tone.c;
	mColorBlob = mColorMinimap & 0x40ffffff;

	for( int lod = 0; lod < getLodCount(); ++lod ) {
		TMeshVector* rms = getRenderMeshes(RM_NORMAL,lod);
		for( int z = 0; z < rms->size(); ++z )
			(*rms)[z]->getParams().addVector4Ref( "vColor", gColors.team[ge.getOwner()].tone.v );
	}

	mOnGround = (etype==ENTITY_NEEDLE) || (etype==ENTITY_NEUROC);
	mOnAir = (etype!=ENTITY_COLLECTOR) && (etype!=ENTITY_CONTAINER);
	mOnSine = (etype==ENTITY_BLOCKER);

	if( etype == ENTITY_NEEDLE ) {
		mHealthBarDY =  1.3f;
		mBlobDY = 0.5f;
		mOutlineDY = 0.6f;
	} else if( etype == ENTITY_BLOCKER ) {
		mHealthBarDY = 1.0f;
		mBlobDY = -0.4f;
		mOutlineDY = 0.3f;
	} else {
		mHealthBarDY = 0.35f;
		mBlobDY = -0.4f;
		mOutlineDY = 0.0f;
	}

	if( mOnSine )
		mBaseAltitude = -0.6f;
	else if( mOnAir )
		mBaseAltitude = gRandom.getFloat( -0.2f, 0.2f );
	else
		mBaseAltitude = gRandom.getFloat( -0.6f, -0.4f );

	/*
	int i;
	int n = re.getAliveTurns();


	SVector3* pos0 = new SVector3[n];
	SVector3* pos1 = new SVector3[n];

	const float RADIUS = 0.7f;

	int bornTurn = re.getBornTurn();

	// init the positions, adjust for no-collisions
	for( i = 0; i < n; ++i ) {
		const CReplayEntity::SState& s = re.getTurnState( i + bornTurn );
		const CGameMap::SCell& cell = gmap.getCell(s.posx,s.posy);
		float hgt = cell.height;
		if( i != 0 || onGround || onSine )
			hgt *= height;
		pos0[i].set( float(s.posx), hgt, float(-s.posy) );
		if( !onGround ) {
			float phi = i * 0.05f;
			pos0[i].x += cosf(phi) * 0.03f;
			pos0[i].z += sinf(phi) * 0.03f;
		}

		if( cell.nearBone )
			levelMesh.fitSphere( pos0[i], RADIUS );
	}

	// go and "smooth out" the positions
	pos1[0] = pos0[0]; // leave first and last intact
	pos1[n-1] = pos0[n-1];
	for( i = 1; i < n-1; ++i ) { 
		pos1[i] = (pos0[i-1] + pos0[i] + pos0[i+1])/3.0f;
		//levelMesh.fitSphere( pos1[i], RADIUS );
	}
	if( onGround ) {
		pos0[0].y = gmap.getCell(pos0[0].x,-pos0[0].z).height * (-1.0f);
		pos0[n-1].y = gmap.getCell(pos0[n-1].x,-pos0[n-1].z).height * (-1.0f);
	}
	for( i = 1; i < n-1; ++i ) { 
		pos0[i] = (pos1[i-1] + pos1[i] + pos1[i+1])/3.0f;
		int cellx = round( pos0[i].x-0.5f );
		int celly = round( -pos0[i].z-0.5f );
		const CGameMap::SCell& cell = gmap.getCell(cellx,celly);
		if( cell.nearBone )
			levelMesh.fitSphere( pos0[i], RADIUS );
		if( onGround ) {
			pos0[i].y = cell.height * (-1.0f);
		}
	}

	mPositions = pos0;
	delete[] pos1;
	*/

	// sounds
	mSndAttack = new CSound( *RGET_SOUND(CSoundDesc("Attack",false)) );
	mSndAttack->setLooping( true );

	mSndBirth = new CSound( *RGET_SOUND(CSoundDesc((etype==ENTITY_NEEDLE||etype==ENTITY_BLOCKER)?"Build":"Spawn",false)) );

	if( ai ) {
		mSndInjured = new CSound( *RGET_SOUND(CSoundDesc("Alarm",false)) );
		mSndInjured->setMinDist( 150.0f );
		mSndInjured->setMaxDist( 300.0f );
	} else
		mSndInjured = 0;
}

CActorEntity::~CActorEntity()
{
	//delete[] mPositions;
	delete mSndAttack;
	delete mSndBirth;
	safeDelete( mSndInjured );
}


/*
SVector3 CActorEntity::samplePos( float t ) const
{
	int n = mReplayEntity->getAliveTurns();
	t -= mReplayEntity->getBornTurn();

	int turn = (int)t;
	int turn0 = turn-1;
	if( turn0 < 0 ) turn0 = 0; else if( turn0 >= n ) turn0 = n-1;
	int turn1 = turn;
	if( turn1 < 0 ) turn1 = 0; else if( turn1 >= n ) turn1 = n-1;
	int turn2 = turn+1;
	if( turn2 < 0 ) turn2 = 0; else if( turn2 >= n ) turn2 = n-1;
	int turn3 = turn+2;
	if( turn3 < 0 ) turn3 = 0; else if( turn3 >= n ) turn3 = n-1;
	float alpha = t-turn;

	SVector3 pos;
	//D3DXVec3Lerp( &pos, &mPositions[turn1], &mPositions[turn2], alpha );
	D3DXVec3CatmullRom( &pos, &mPositions[turn0], &mPositions[turn1], &mPositions[turn2], &mPositions[turn3], alpha );
	return pos;
}
*/


void CActorEntity::update()
{
	//float t = CGameInfo::getInstance().getTime();
	
	bool alive = mGameEntity->isAlive();
	if( alive ) {
		// fade out the outline
		float dt = CSystemTimer::getInstance().getDeltaTimeS();
		mOutlineTTL -= dt;
		if( mOutlineTTL < 0.0f )
			mOutlineTTL = 0.0f;

		//float t0 = t + 2.1f;

		SMatrix4x4& m = mWorldMat;
		/*SVector3 pos = samplePos( t );
		SVector3 dir = samplePos( t0 ) - pos;
		if( dir.lengthSq() < 1.0e-6f )
			dir = m.getAxisZ();
		else
			dir.normalize();
		*/
		// TBD
		const CGameEntity::SState& st = mGameEntity->getState();
		SVector3 pos = SVector3(st.posx,0,st.posy);
		SVector3 dir(0,0,1);

		if( mGameEntity->getType() == ENTITY_BLOCKER ) {
			double tt = CSystemTimer::getInstance().getTimeS();
			D3DXMatrixRotationY( &m, tt * 0.2f );
			m.getOrigin() = pos;
			m.getOrigin().y += sinf( tt * 0.6f ) * 0.2f;
		} else {
			m.getOrigin() = pos;
			m.getAxisZ() = dir;
			m.getAxisZ().y *= 0.2f;
			m.getAxisZ().normalize();
			m.getAxisY().set( 0, 1, 0 );
			m.getAxisX() = m.getAxisY().cross( m.getAxisZ() );
			m.getAxisX().normalize();
			m.getAxisY() = m.getAxisZ().cross( m.getAxisX() );
		}
	} else {
		mOutlineTTL = 0.0f;
	}

	// sounds
	updateSounds( !alive );
}

void CActorEntity::updateSounds( bool dead )
{
	// only play sounds in normal play mode
	if( !gPlayMode || dead ) {
		mSndAttack->stop();
		return;
	}

	// TBD
	/*
	float t = CGameInfo::getInstance().getTime();
	int turn = int(t);
	float turnA = t - turn;

	const CReplayEntity::SState& sPrev = mReplayEntity->getTurnState( turn-1 );
	const CReplayEntity::SState& sNow = mReplayEntity->getTurnState( turn );

	const float volume = gAppSettings.soundVolume * 0.01f;

	// attack sound
	mSndAttack->setTransform( mWorldMat );
	mSndAttack->setMinDist( gAppSettings.followMode ? 10.0f : gAppSettings.megaZoom );
	mSndAttack->setMaxDist( gAppSettings.followMode ? 100.0f : gAppSettings.megaZoom * 3 );
	if( sNow.state == ENTSTATE_ATTACK ) {
		
		// attacking now. either fade in if was not attacking, or leave
		if( sPrev.state != ENTSTATE_ATTACK )
			mSndAttack->setVolume( turnA * volume );
		else
			mSndAttack->setVolume( volume );
		mSndAttack->startOrUpdate();
	} else {
		// not attacking now. either stop if was not attacking, or fade out
		if( sPrev.state != ENTSTATE_ATTACK )
			mSndAttack->stop();
		else {
			mSndAttack->setVolume( (1.0f - turnA) * volume );
			mSndAttack->startOrUpdate();
		}
	}

	// birth sound
	mSndBirth->setMinDist( gAppSettings.followMode ? 1.0f : gAppSettings.megaZoom );
	mSndBirth->setMaxDist( gAppSettings.followMode ? 30.0f : gAppSettings.megaZoom * 3 );
	mSndBirth->setVolume( (gAppSettings.followMode ? 1.0f : 0.7f) * volume );
	if( turn == mReplayEntity->getBornTurn() ) {
		if( !mSndBirth->isPlaying() ) {
			mSndBirth->setTransform( mWorldMat );
			mSndBirth->start();
		}
	}

	// injured?
	if( mSndInjured && !mSndInjured->isPlaying() ) {
		if( sNow.health < sPrev.health ) {
			mSndInjured->setTransform( mWorldMat );
			mSndInjured->start();
		}
	}
	*/
}

