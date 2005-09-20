#include "stdafx.h"
#include "GameEntity.h"

static const char* ENT_TYPE_NAMES[ENTITYCOUNT] = {
	"Needle",
	"Explorer",
	"Collector",
	"AI",
	"WhiteCell", // TBD: Container
	"BlackCell", // TBD: NeuroCtrl
	"Blocker",
};

CGameEntity::CGameEntity( unsigned short eid, eEntityType type, int owner, int bornTurn )
:	mLastUpdateTurn( bornTurn-1 )
,	mID( eid )
,	mType( type )
,	mOwner( owner )
,	mMaxHealth( -1 )
,	mBornTurn( bornTurn )
{
	assert( type >= 0 && type < ENTITYCOUNT );
	assert( bornTurn >= 0 );

	mTypeName = ENT_TYPE_NAMES[type];
	memset( &mState, 0, sizeof(SState) );
}

CGameEntity::~CGameEntity()
{
}

void CGameEntity::updateState( int turn, const SState& state )
{
	if( turn == mLastUpdateTurn )
		return; // already updated this turn

	// this is a very first update, remember max. health
	if( mLastUpdateTurn == mBornTurn-1 ) {
		mMaxHealth = state.health;
	}

	mState = state;

	mLastUpdateTurn = turn;
}

void CGameEntity::markDead()
{
	// do nothing if already dead
	if( mState.state == ENTSTATE_DEAD )
		return;

	mState.state = ENTSTATE_DEAD;
}
