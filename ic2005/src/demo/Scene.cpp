#include "stdafx.h"
#include "Scene.h"
#include "ComplexStuffEntity.h"
#include "ControllableCharacter.h"
#include "ThirdPersonCamera.h"
#include "SceneShared.h"
#include "wallz/FractureScenario.h"
#include <dingus/math/Interpolation.h>


// --------------------------------------------------------------------------

class CRoomObjectEntity : public CMeshEntity {
public:
	CRoomObjectEntity( const std::string& name )
		:	CMeshEntity(name), mMoved( true )
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

	void	setMoved() { mMoved = true; }
	void	update() {
		if( !mMoved )
			return;
		mMoved = false;

		// TBD: don't do full inverse
		D3DXMatrixInverse( &mInvWorld, NULL, &mWorldMat );
	}

	void	render( eRenderMode renderMode )
	{
		// update light/eye positions
		D3DXVec3TransformCoord( &mLightPosOS, &gSLightPos, &mInvWorld );
		D3DXVec3TransformCoord( &mEyePosOS, &G_RENDERCTX->getCamera().getEye3(), &mInvWorld );
		
		CMeshEntity::render( renderMode );
	}


private:
	SMatrix4x4	mInvWorld;
	SVector3	mLightPosOS;	// light pos in object space
	SVector3	mEyePosOS;		// eye pos in object space
	bool		mMoved;
};

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

#include <dingus/lua/LuaSingleton.h>
#include <dingus/lua/LuaHelper.h>
#include <dingus/lua/LuaIterator.h>


static bool gReadScene( const char* fileName, std::vector<CRoomObjectEntity*>& scene )
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

static const float BED_FRACTURE_FRAME = 831 + 800;

static const float STONE_SHOW_FRAME = 844 + 800;
static const float STONE_BEGIN_FRAME = 1115 + 800;
static const float STONE_HIDE_FRAME = 1710 + 800;

static const float WALL_LOD1_FRAME = 3000 + 800;

static const float DOOR_BEGIN_FRAME = 5304 + 800;
static const float DOOR_END_FRAME = 5400 + 800;

static const float GUY2_BEGIN_FRAME = 5500;
static const float GUY3_BEGIN_FRAME = 5639;


CSceneMain::CSceneMain( CSceneSharedStuff* sharedStuff )
:	mSharedStuff( sharedStuff )
,	mAnimFrameCount(0)
,	mAnimDuration(0)
,	mCurrAnimFrame(0)
,	mCurrAnimAlpha(0)
{
	// characters
	mCharacter = new CComplexStuffEntity( "Bicas", "BicasAnim" );
	addAnimEntity( *mCharacter );
	mCharacter2 = new CComplexStuffEntity( "Bicas", "Bicas2Anim" );
	addAnimEntity( *mCharacter2 );
	mCharacter3 = new CComplexStuffEntity( "Bicas", "Bicas3Anim" );
	addAnimEntity( *mCharacter3 );

	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// bed/stone
	mBedStatic = new CMeshEntity( "Bed" );
	addEntity( *mBedStatic );
	mBedAnim = new CComplexStuffEntity( "BedPieces", "BedAnim" );
	addAnimEntity( *mBedAnim );
	mStone = new CComplexStuffEntity( "Stone", "StoneAnim" );
	addAnimEntity( *mStone );

	// room
	gReadScene( "data/scene.lua", mRoom );

	// fracture scenario
	gReadFractureScenario( "data/fractures.txt" );

	// camera anim
	mCameraAnim = RGET_ANIM("Camera");
	mAnimDuration = gGetAnimDuration( *mCameraAnim, false );
	mCameraAnimPos = mCameraAnim->findVector3Anim("pos");
	mCameraAnimRot = mCameraAnim->findQuatAnim("rot");
	mCameraAnimParams = mCameraAnim->findVector3Anim("cam");
	mAnimFrameCount = mCameraAnimPos->getLength();

	// start anims
	//const time_value HACK_START_ANIM = time_value::fromsec( -40.0f );
	const time_value HACK_START_ANIM = time_value::fromsec( 0.0f );

	mCharacter->getAnimator().playDefaultAnim( HACK_START_ANIM );
	mBedAnim->getAnimator().playDefaultAnim( time_value() );
	mStone->getAnimator().playDefaultAnim( time_value() );
}


CSceneMain::~CSceneMain()
{
	stl_utils::wipe( mRoom );
}


static const float CAM_C0_FRAMES[] = {
	-619, -476, -82,
	522, 652, 780, 1206, 1294, 1437,
	1540, 1830, 2220,
	2312, 2647, 2712, 2819, 2872,
	2986, 3168, 3302, 3366, 3397, 3489,
	3669, 3759, 3852, 3962, 4033, 4177, 4310,
	4447, 4621, 4726, 4850, 5035, 5105, 5213,
	5304, 5402, 5497, 5704,
};
static const int CAM_C0_FRAMES_SIZE = sizeof(CAM_C0_FRAMES) / sizeof(CAM_C0_FRAMES[0]);
static const int CAM_C0_ADD = 800;



void CSceneMain::animateCamera()
{
	getCamera().mWorldMat.identify();
	getCamera().mWorldMat.getOrigin().set( 0, 1.0f, -3.0f );


	SVector3 camPos;
	SQuaternion camRot;
	SVector3 camParams;

	int c0idx = -1;
	for( int i = 0; i < CAM_C0_FRAMES_SIZE; ++i ) {
		float fr = CAM_C0_FRAMES[i]+CAM_C0_ADD;
		if( mCurrAnimFrame >= fr-2 && mCurrAnimFrame <= fr ) {
			c0idx = i;
			break;
		}
	}
	if( c0idx < 0 ) {
		mCameraAnimPos->sample( mCurrAnimAlpha, 0, 1, &camPos );
		mCameraAnimRot->sample( mCurrAnimAlpha, 0, 1, &camRot );
		mCameraAnimParams->sample( mCurrAnimAlpha, 0, 1, &camParams );
	} else {
		SVector3 pos1, pos2;
		SQuaternion rot1, rot2;
		SVector3 params1, params2;
		double a1 = mCurrAnimAlpha - (3.0/mAnimFrameCount);
		double a2 = mCurrAnimAlpha - (2.5/mAnimFrameCount);
		double lerper = (mCurrAnimAlpha-a1) / (a2-a1);
		mCameraAnimPos->sample( a1, 0, 1, &pos1 );
		mCameraAnimPos->sample( a2, 0, 1, &pos2 );
		mCameraAnimRot->sample( a1, 0, 1, &rot1 );
		mCameraAnimRot->sample( a2, 0, 1, &rot2 );
		mCameraAnimParams->sample( a1, 0, 1, &params1 );
		mCameraAnimParams->sample( a2, 0, 1, &params2 );
		camPos = math_lerp<SVector3>( pos1, pos2, lerper );
		camRot = math_lerp<SQuaternion>( rot1, rot2, lerper );
		camParams = math_lerp<SVector3>( params1, params2, lerper );
	}

	const float fov = camParams.z;

	SMatrix4x4 mr;
	D3DXMatrixRotationX( &mr, D3DX_PI/2 );
	getCamera().mWorldMat = mr * SMatrix4x4( camPos, camRot );

	const float camnear = 0.1f; // not from animation, just hardcoded
	const float camfar = 50.0f;

	float aspect = CD3DDevice::getInstance().getBackBufferAspect();
	getCamera().setProjectionParams( fov / aspect, aspect, camnear, camfar );
}



void CSceneMain::update( time_value demoTime, float dt )
{
	mSharedStuff->updatePhysics();

	int n = mRoom.size();
	for( int i = 0; i < n; ++i ) {
		mRoom[i]->update();
	}

	float demoTimeS = demoTime.tosec();
	mCurrAnimAlpha = demoTimeS / mAnimDuration;
	mCurrAnimFrame = mCurrAnimAlpha * mAnimFrameCount;

	mCharacter->update( demoTime );

	if( mCurrAnimFrame >= GUY2_BEGIN_FRAME ) {
		double animS = (mCurrAnimFrame-GUY2_BEGIN_FRAME)/ANIM_FPS;
		if( animS < 0 ) animS = 0;
		time_value animTime = time_value::fromsec( animS );
		mCharacter2->update( animTime );
	}
	if( mCurrAnimFrame >= GUY3_BEGIN_FRAME ) {
		double animS = (mCurrAnimFrame-GUY3_BEGIN_FRAME)/ANIM_FPS;
		if( animS < 0 ) animS = 0;
		time_value animTime = time_value::fromsec( animS );
		mCharacter3->update( animTime );
	}
	

	if( mCurrAnimFrame >= BED_FRACTURE_FRAME ) {
		double bedAnimS = (mCurrAnimFrame-BED_FRACTURE_FRAME)/ANIM_FPS;
		time_value bedAnimTime = time_value::fromsec( bedAnimS );
		mBedAnim->update( bedAnimTime );
	}
	if( mCurrAnimFrame >= STONE_SHOW_FRAME && mCurrAnimFrame <= STONE_HIDE_FRAME ) {
		double stoneAnimS = (mCurrAnimFrame-STONE_BEGIN_FRAME)/ANIM_FPS;
		if( stoneAnimS < 0.0 )
			stoneAnimS = 0.0;
		time_value stoneAnimTime = time_value::fromsec( stoneAnimS );
		mStone->update( stoneAnimTime );
	}

	int wallsLod = mCurrAnimFrame < WALL_LOD1_FRAME ? 0 : 1;

	gUpdateFractureScenario( mCurrAnimFrame, demoTimeS, wallsLod, mSharedStuff->getWalls(wallsLod) );

	mSharedStuff->updateFracture( wallsLod, demoTimeS );

	animateCamera();
}


void CSceneMain::render( eRenderMode renderMode )
{
	int wallsLod = mCurrAnimFrame < WALL_LOD1_FRAME ? 0 : 1;
	mSharedStuff->renderWalls( wallsLod, renderMode );

	// characters
	mCharacter->render( renderMode );
	if( mCurrAnimFrame >= DOOR_BEGIN_FRAME ) {
		mCharacter2->render( renderMode );
		mCharacter3->render( renderMode );
	}

	// room
	int i, n;
	n = mRoom.size();
	for( i = 0; i < n; ++i ) {
		mRoom[i]->render( renderMode );
	}

	// bed
	if( mCurrAnimFrame < BED_FRACTURE_FRAME )
		mBedStatic->render( renderMode );
	else
		mBedAnim->render( renderMode );

	// stone
	if( mCurrAnimFrame > STONE_SHOW_FRAME && mCurrAnimFrame < STONE_HIDE_FRAME )
		mStone->render( renderMode );
}

const SMatrix4x4* CSceneMain::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
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
	mSharedStuff->renderWalls( 0, renderMode );
	
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
