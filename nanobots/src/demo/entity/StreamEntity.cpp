#include "stdafx.h"
#include "StreamEntity.h"
#include "../GameInfo.h"
#include "../game/GameDesc.h"
#include <dingus/utils/Random.h>



CStreamEntity::CStreamEntity( const CGameMap::SStream& stream, float x, float y )
:	CMeshEntity( "StreamEntity", 1 )
,	mStream( &stream )
,	mVelocityX(stream.deltaX + gRandom.getFloat(-0.25f,0.25f))
,	mVelocityY(stream.deltaY + gRandom.getFloat(-0.25f,0.25f))
{
	const CGameMap& gmap = CGameInfo::getInstance().getGameDesc().getMap();

	D3DXMatrixRotationY( &mWorldMat, gRandom.getFloat(0,D3DX_PI*2) );
	mWorldMat.getOrigin().x =  x;
	mWorldMat.getOrigin().z =  -y;
	mWorldMat.getOrigin().y = gRandom.getFloat( -1.0f, 1.0f );

	mColor.set( 1.0f, 1.0f, 1.0f, 0.2f );
	
	if( getRenderMeshes( RM_NORMAL, 0 ) ) {
		(*getRenderMeshes( RM_NORMAL, 0 ))[0]->getParams().addVector4Ref( "vColor", mColor );
	}
}


CStreamEntity::~CStreamEntity()
{
}

void CStreamEntity::update()
{
	float dt = CSystemTimer::getInstance().getDeltaTimeS();
	float x = mWorldMat.getOrigin().x + mVelocityX * dt;
	while( x < mStream->x )
		x += mStream->width;
	while( x > mStream->x+mStream->width )
		x -= mStream->width;
	mWorldMat.getOrigin().x = x;

	float y = -mWorldMat.getOrigin().z + mVelocityY * dt;
	while( y < mStream->y )
		y += mStream->height;
	while( y > mStream->y+mStream->height )
		y -= mStream->height;
	mWorldMat.getOrigin().z = -y;

	int cellX = round( mWorldMat.getOrigin().x );
	int cellY = round( -mWorldMat.getOrigin().z );
	const CGameMap::SCell& cell = CGameInfo::getInstance().getGameDesc().getMap().getCell( cellX, cellY );
	if( CGameMap::isBlood( cell.type ) ) {
		mColor.w = cell.nearBone ? 0.1f : 0.2f;
	} else {
		mColor.w = 0.0f;
	}
}

void CStreamEntity::render( eRenderMode renderMode )
{
	if( mColor.w == 0.0f )
		return;
	CMeshEntity::render( renderMode, 0, false );
}
