#include "stdafx.h"
#include "PointEntity.h"
#include "../GameInfo.h"
#include "../game/GameColors.h"
#include "../game/GameReplay.h"
#include <dingus/utils/Random.h>


static const char* POINT_TYPENAMES[PTCOUNT] = {
	"PointAZN",
	"PointHoshimi",
	"PointInjection",
};


CPointEntity::CPointEntity( const CGameMap::SPoint& point )
:	CMeshEntity( POINT_TYPENAMES[ point.type ], 1 ),
	mTimeOffset( gRandom.getFloat(0,10) ),
	mPoint( &point )
{
	const CGameMap& gmap = CGameInfo::getInstance().getGameMap();

	D3DXMatrixRotationY( &mWorldMat, gRandom.getFloat(0,D3DX_PI*2) );
	mWorldMat.getOrigin().x =  point.x;
	mWorldMat.getOrigin().z = -point.y;

	switch( point.type ) {
	case PT_AZN:
	case PT_HOSHIMI:
		mAlphaBase = 0.40f;	mAlphaAmpl = 0.10f;
		mWorldMat.getOrigin().y = -gmap.getCell(point.x,point.y).height;
		mColor = &(D3DXCOLOR(point.colorTone).r);
		break;
	case PT_INJECTION:
		mAlphaBase = 0.20f;	mAlphaAmpl = 0.20f;
		mWorldMat.getAxisY().y = gmap.getCell(point.x,point.y).height;
		mWorldMat.getOrigin().y = 0.0f;
		mColor = &(D3DXCOLOR(point.colorMain).r);
		break;
	}
	
	if( getRenderMeshes( RM_NORMAL, 0 ) ) {
		(*getRenderMeshes( RM_NORMAL, 0 ))[0]->getParams().addVector4Ref( "vColor", mColor );
	}
}


CPointEntity::~CPointEntity()
{
}

void CPointEntity::update()
{
	// if we're Hoshimi - go through all needles and see if any stands on me
	if( mPoint->type == PT_HOSHIMI ) {
		float t = CGameInfo::getInstance().getTime();
		const CGameReplay& replay = CGameInfo::getInstance().getReplay();
		int n = replay.getEntityCount();
		for( int i = 0; i < n; ++i ) {
			const CReplayEntity& e = replay.getEntity(i);
			if( e.getType() != ENTITY_NEEDLE )
				continue;
			if( !e.isAlive( t ) )
				continue;
			const CReplayEntity::SState& s = e.getTurnState(0);
			if( s.posx == mPoint->x && s.posy == mPoint->y ) {
				mColor.w = 0.0f;
				return;
			}
		}
	}
	double t = CSystemTimer::getInstance().getTimeS() - mTimeOffset;
	mColor.w = mAlphaBase + cosf( t + sinf( t * 2.1f ) ) * mAlphaAmpl;
}

void CPointEntity::renderPoint( eRenderMode renderMode )
{
	if( mColor.w == 0.0f )
		return;
	render( renderMode, 0, false );
}
