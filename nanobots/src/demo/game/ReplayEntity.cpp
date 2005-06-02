#include "stdafx.h"
#include "ReplayEntity.h"

static const char* ENT_TYPE_NAMES[ENTITYCOUNT] = {
	"Needle",
	"Explorer",
	"Collector",
	"AI",
	"CureBot",
	"Explorer", // TBD: pilot
	"Blocker",
	"WhiteCell",
	"BlackCell",
};

CReplayEntity::CReplayEntity( eEntityType type, int owner, int bornTurn, int deathTurn, const SState* states )
:	mType( type ), mOwner( owner ),
	mBornTurn( bornTurn ), mDeathTurn( deathTurn ), mAliveTurns( deathTurn-bornTurn+1 )
{
	assert( type >= 0 && type < ENTITYCOUNT );
	assert( bornTurn >= 0 );
	assert( deathTurn >= bornTurn );
	assert( mAliveTurns >= 1 );

	mTypeName = ENT_TYPE_NAMES[type];

	mStates = new SState[mAliveTurns];
	memcpy( mStates, states, mAliveTurns*sizeof(SState) );

	mMaxHealth = mStates[0].health;
}

CReplayEntity::~CReplayEntity()
{
	delete[] mStates;
}
