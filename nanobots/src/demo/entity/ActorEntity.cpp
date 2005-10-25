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
}

CActorEntity::~CActorEntity()
{
	//delete[] mPositions;
}


SVector3 CActorEntity::samplePos( float timeAlpha ) const
{
	const CGameEntity::SState& st0 = mGameEntity->getState();
	const CGameEntity::SState& st1 = mGameEntity->getStateCurr();

	SVector3 pos;
	D3DXVec3Lerp( &pos, &st0.pos, &st1.pos, timeAlpha );
	return pos;
}


void CActorEntity::update( float timeAlpha )
{
	bool alive = mGameEntity->isAlive();
	if( alive ) {
		// fade out the outline
		float dt = CSystemTimer::getInstance().getDeltaTimeS();
		mOutlineTTL -= dt;
		if( mOutlineTTL < 0.0f )
			mOutlineTTL = 0.0f;

		SMatrix4x4& m = mWorldMat;
		SVector3 pos = samplePos( timeAlpha );
		SVector3 dir = samplePos( timeAlpha + 0.1f ) - pos;
		if( dir.lengthSq() < 1.0e-3f )
			dir = m.getAxisZ();
		else
			dir.normalize();

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
}
