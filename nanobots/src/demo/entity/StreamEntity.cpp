#include "stdafx.h"
#include "StreamEntity.h"
#include "../GameInfo.h"
#include "../game/GameDesc.h"
#include "../StreamImpostorsRenderer.h"
#include <dingus/utils/Random.h>



CStreamEntity::CStreamEntity( const CGameMap::SStream& stream, float x, float y, int type )
:	CMeshEntity( type==0 ? "StreamWhite" : "StreamRed", 1 )
,	mStream( &stream )
,	mVelocityX(stream.deltaX*1.5f + gRandom.getFloat(-0.5f,0.5f))
,	mVelocityY(stream.deltaY*1.5f + gRandom.getFloat(-0.5f,0.5f))
,	mType(type)
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
		mColor.w = 0.2f;
	} else {
		mColor.w = 0.0f;
	}
}

void CStreamEntity::render( eRenderMode renderMode, CStreamImpostorsRenderer& impostorer, bool insideView )
{
	// if we aren't visible anyway - exit
	if( mColor.w == 0.0f )
		return;

	// determine distance to camera to blend between real model and impostor
	// in "inside view", just fade out
	float distSq = SVector3( G_RENDERCTX->getCamera().getEye3() - mWorldMat.getOrigin() ).lengthSq();
	if( insideView ) {
		const float FADE_START = 15.0f;
		const float FADE_LEN2 = 400.0f;
		float alpha = clamp( 1.0f - (distSq - FADE_START*FADE_START)/FADE_LEN2, 0.0f, 1.0f );
		if( alpha > 1.0f/255.0f ) {
			mColor.w *= alpha;
			CMeshEntity::render( renderMode, 0, false );
		}
	} else {
		const float FADE_START = 25.0f;
		const float FADE_LEN2 = 600.0f;
		float alpha = clamp( 1.0f - (distSq - FADE_START*FADE_START)/FADE_LEN2, 0.0f, 1.0f );
		if( alpha < 254.0f/255.0f ) {
			impostorer.addEntity( mWorldMat, clamp(mColor.w*2*(1-alpha),0,1), mType );
		}
		if( alpha > 1.0f/255.0f ) {
			mColor.w *= alpha;
			CMeshEntity::render( renderMode, 0, false );
		}
	}
}
