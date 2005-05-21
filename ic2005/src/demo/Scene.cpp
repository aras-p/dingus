#include "stdafx.h"
#include "Scene.h"
#include "ComplexStuffEntity.h"

#include <dingus/lua/LuaSingleton.h>
#include <dingus/lua/LuaHelper.h>
#include <dingus/lua/LuaIterator.h>


// --------------------------------------------------------------------------

CRoomObjectEntity::CRoomObjectEntity( const std::string& name )
:	CMeshEntity(name,name), mMoved( true )
{
	// add to effect parameters
	for( int i = 0; i < RMCOUNT; ++i ) {
		CRenderableMesh* rmesh = getRenderMesh( eRenderMode(i) );
		if( !rmesh )
			continue;
		rmesh->getParams().addVector3Ref( "vLightPosOS", mLightPosOS );
		rmesh->getParams().addVector3Ref( "vEyeOS", mEyePosOS );
	}
}

void CRoomObjectEntity::update( const SVector3& wlightPos )
{
	if( !mMoved )
		return;
	mMoved = false;

	// TBD: don't do full inverse
	D3DXMatrixInverse( &mInvWorld, NULL, &mWorldMat );

	// update light position
	D3DXVec3TransformCoord( &mLightPosOS, &wlightPos, &mInvWorld );
}

void CRoomObjectEntity::render( eRenderMode renderMode )
{
	// if reflection - check whether we're not in front of reflection plane
	if( renderMode == RM_REFLECTED ) {
		SVector3 centerPt;
		D3DXVec3TransformCoord( &centerPt, &getMesh().getTotalAABB().getCenter(), &mWorldMat );
		if( gReflPlane.distance( centerPt ) < 0.0f )
			return;
	}

	// update eye position
	D3DXVec3TransformCoord( &mEyePosOS, &G_RENDERCTX->getCamera().getEye3(), &mInvWorld );
	
	CMeshEntity::render( renderMode );
}


// --------------------------------------------------------------------------

CScene::CScene()
{
	mCamera.mWorldMat.identify();
}


CScene::~CScene()
{
	stl_utils::wipe( mEntities );
	stl_utils::wipe( mAnimEntities );
}


// --------------------------------------------------------------------------



bool gReadScene( const char* fileName, std::vector<CRoomObjectEntity*>& scene )
{
	CLuaSingleton& lua = CLuaSingleton::getInstance();
	int errorCode = lua.doFile( fileName, false );
	if( errorCode )
		return false;
	
	// iterate scene table
	CLuaValue luaScene = lua.getGlobal("scene");
	CLuaArrayIterator itScene( luaScene );
	while( itScene.hasNext() ) {
		CLuaValue& luaObj = itScene.next();

		std::string name = CLuaHelper::getString( luaObj, "name" );
		SVector3 pos = CLuaHelper::getVector3( luaObj, "pos" );
		SQuaternion rot = CLuaHelper::getQuaternion( luaObj, "rot" );

		CRoomObjectEntity* obj = new CRoomObjectEntity( name );
		obj->mWorldMat = SMatrix4x4( pos, rot );
		obj->setMoved();
		scene.push_back( obj );
	}
	luaScene.discard();

	return true;
}

