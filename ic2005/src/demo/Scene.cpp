#include "stdafx.h"
#include "Scene.h"
#include "ControllableCharacter.h"
#include "ThirdPersonCamera.h"
#include "SceneShared.h"

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

void CRoomObjectEntity::update()
{
	if( !mMoved )
		return;
	mMoved = false;

	// TBD: don't do full inverse
	D3DXMatrixInverse( &mInvWorld, NULL, &mWorldMat );
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

	// update light/eye positions
	D3DXVec3TransformCoord( &mLightPosOS, &gSLightPos, &mInvWorld );
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

		pos.x += ROOM_MID.x;
		pos.z += ROOM_MID.z;

		CRoomObjectEntity* obj = new CRoomObjectEntity( name );
		obj->mWorldMat = SMatrix4x4( pos, rot );
		obj->setMoved();
		scene.push_back( obj );
	}
	luaScene.discard();

	return true;
}


// --------------------------------------------------------------------------

CSceneInteractive::CSceneInteractive( CSceneSharedStuff* sharedStuff )
:	mSharedStuff( sharedStuff )
{
	const float WALK_BOUNDS = 0.9f;
	mCharacter = new CControllableCharacter( ROOM_MIN.x+WALK_BOUNDS, ROOM_MIN.z+WALK_BOUNDS, ROOM_MAX.x-WALK_BOUNDS, ROOM_MAX.z-WALK_BOUNDS );
	addAnimEntity( *mCharacter );

	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// room
	gReadScene( "data/scene.lua", mRoom );

	const float CAMERA_BOUND = 0.15f;
	SVector3 CAMERA_BOUND_MIN = ROOM_MIN + SVector3(CAMERA_BOUND,CAMERA_BOUND,CAMERA_BOUND);
	SVector3 CAMERA_BOUND_MAX = ROOM_MAX - SVector3(CAMERA_BOUND,CAMERA_BOUND,CAMERA_BOUND);
	mCamController = new CThirdPersonCameraController( mCharacter->getWorldMatrix(), getCamera().mWorldMat, CAMERA_BOUND_MIN, CAMERA_BOUND_MAX );
}

CSceneInteractive::~CSceneInteractive()
{
	stl_utils::wipe( mRoom );
	delete mCamController;
}


void CSceneInteractive::update( time_value demoTime, float dt )
{
	mSharedStuff->updatePhysics();

	int n = mRoom.size();
	for( int i = 0; i < n; ++i ) {
		mRoom[i]->update();
	}
	
	float demoTimeS = demoTime.tosec();

	mCharacter->update( demoTime );

	mSharedStuff->updateFracture( 0, demoTimeS );

	mCamController->update( dt );
	const float camnear = 0.1f;
	const float camfar = 50.0f;
	const float camfov = D3DX_PI/4;
	getCamera().setProjectionParams( camfov, CD3DDevice::getInstance().getBackBufferAspect(), camnear, camfar );
}


void CSceneInteractive::render( eRenderMode renderMode )
{
	//mSharedStuff->renderWalls( 0, renderMode );
	
	mCharacter->render( renderMode );
	
	int i, n;
	n = mRoom.size();
	for( i = 0; i < n; ++i ) {
		mRoom[i]->render( renderMode );
	}
}


void CSceneInteractive::processInput( float mov, float rot, bool attack, time_value demoTime )
{
	mCharacter->move( mov, demoTime );
	mCharacter->rotate( rot );
	if( attack ) {
		mCharacter->attack( demoTime );
	}
}

const SMatrix4x4* CSceneInteractive::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
