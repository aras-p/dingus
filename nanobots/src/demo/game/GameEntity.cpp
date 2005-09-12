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
:	mID( eid )
,	mType( type )
,	mOwner( owner )
,	mMaxHealth( -1 )
,	mBornTurn( bornTurn )
{
	assert( type >= 0 && type < ENTITYCOUNT );
	assert( bornTurn >= 0 );

	mTypeName = ENT_TYPE_NAMES[type];
}

CGameEntity::~CGameEntity()
{
}
