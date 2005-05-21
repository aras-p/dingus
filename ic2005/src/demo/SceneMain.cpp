#include "stdafx.h"
#include "SceneMain.h"
#include "ComplexStuffEntity.h"
#include "SceneShared.h"
#include "wallz/FractureScenario.h"
#include <dingus/math/Interpolation.h>
#include <dingus/math/MathUtils.h>


// --------------------------------------------------------------------------

static const float BED_FRACTURE_FRAME = 831 + 800;
static const float BED_HIDE_FRAME = 1710 + 800;

static const float STONE_SHOW_FRAME = 844 + 800;
static const float STONE_BEGIN_FRAME = 1115 + 800;
static const float STONE_HIDE_FRAME = 1710 + 800;

static const float WALL_LOD1_FRAME = 3000 + 800;

static const float FLOOR_FR_FRAME = 3800 + 950;

static const float DOOR_BEGIN_FRAME = 5304 + 800;
static const float DOOR_END_FRAME = 5400 + 800;

static const float GUY2_BEGIN_FRAME = 5500 + 800;
static const float GUY3_BEGIN_FRAME = 5639 + 800;

static const float TIMEBLEND_BEGIN_FRAME = 2450 + 950;
static const float TIMEBLEND_END_FRAME = 3770 + 950;


static const float ROOM2_BEGIN_FRAME = 5304 + 800;
static const float LIGHT_SWITCH_FRAME = 5402 + 800;

static const float ATK2_BEGIN_FRAME = 3160 + 800;
static const float ATK2_END_FRAME = 3230 + 800;



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
	mCharacter2->setLightPos( LIGHT_POS_2 );
	addAnimEntity( *mCharacter2 );
	mCharacter3 = new CComplexStuffEntity( "Bicas", "Bicas3Anim" );
	mCharacter3->setLightPos( LIGHT_POS_2 );
	addAnimEntity( *mCharacter3 );

	mSpineBoneIndex = mCharacter->getAnimator().getCurrAnim()->getCurveIndexByName( "Spine" );

	// bed/stone
	mBedStatic = new CMeshEntity( "Bed" );
	mBedStatic->addLightToParams( LIGHT_POS_1 );
	addEntity( *mBedStatic );
	mBedAnim = new CComplexStuffEntity( "BedPieces", "BedAnim" );
	addAnimEntity( *mBedAnim );
	mStone = new CComplexStuffEntity( "Stone", "StoneAnim" );
	addAnimEntity( *mStone );

	// attacks
	mAttack2_1 = new CComplexStuffEntity( "AttackFx2_1", "AttackFx2" );
	addAnimEntity( *mAttack2_1 );
	mAttack2_2 = new CComplexStuffEntity( "AttackFx2_2", "AttackFx2" );
	addAnimEntity( *mAttack2_2 );

	// rooms
	gReadScene( "data/scene.lua", mRoom );
	gReadScene( "data/scene2.lua", mRoom2 );

	mRoom2Top = new CMeshEntity( "Room2Top" );
	mRoom2Top->addLightToParams( LIGHT_POS_2 );
	addEntity( *mRoom2Top );
	mRoom2Bottom = new CMeshEntity( "Room2Bottom" );
	mRoom2Top->addLightToParams( LIGHT_POS_2 );
	addEntity( *mRoom2Bottom );

	// animating doors
	CAnimationBunch* animDoors = RGET_ANIM("DoorsAnim");
	mDoorsAnim = new CCharacterAnimator();
	mDoorsAnim->setDefaultAnim( *animDoors, gGetAnimDuration(*animDoors,false), 0.1f );
	mDoorsAnim->playDefaultAnim( time_value() );
	mDoorsAnim->updateLocal( time_value() );
	mDoorsAnim->updateWorld();
	// find the room entity for each door animation curve
	// we can't match by name because some names in entities are duplicated :)
	// so we try to match by 'good enough' correspondence in transform
	int ndoorEnts = animDoors->getCurveCount();
	int nroomEnts = mRoom.size();
	mDoorAnim2RoomIdx.resize( ndoorEnts, -1 );
	for( int i = 0; i < ndoorEnts; ++i ) {
		// we must match names, if we throw out last letter from anim name
		std::string doorName = animDoors->getCurveName(i);
		doorName.resize( doorName.size()-1 );

		const SMatrix4x4& doorMat = mDoorsAnim->getBoneWorldMatrices()[i];

		for( int j = 0; j < nroomEnts; ++j ) {
			const CMeshEntity& re = *mRoom[j];
			if( doorName != re.getDescName() )
				continue;
			
			const SMatrix4x4& roomMat = re.mWorldMat;

			// it's enough just to match position roughly
			// name filter earlier rejected all the rest
			const float MATCH_POS = 0.01f;
			if( SVector3(roomMat.getOrigin()-doorMat.getOrigin()).lengthSq() < MATCH_POS )
			{
				// match!
				mDoorAnim2RoomIdx[i] = j;
				break;
			}
		}
	}

	// fracture scenario
	gReadFractureScenario( "data/fractures.txt" );

	// camera anim
	mCameraAnim = RGET_ANIM("Camera");
	mAnimDuration = gGetAnimDuration( *mCameraAnim, false );
	mCameraAnimPos = mCameraAnim->findVector3Anim("pos");
	mCameraAnimRot = mCameraAnim->findQuatAnim("rot");
	mCameraAnimParams = mCameraAnim->findVector3Anim("cam");
	mAnimFrameCount = mCameraAnimPos->getLength();
	// DOF anim
	mDOFAnim = RGET_ANIM("Dof");
	mDOFAnimPos = mDOFAnim->findVector3Anim("pos");
	mDOFAnimRange = mDOFAnim->findFloatAnim("scale");

	// start anims
	mCharacter->getAnimator().playDefaultAnim( time_value() );
	mBedAnim->getAnimator().playDefaultAnim( time_value() );
	mStone->getAnimator().playDefaultAnim( time_value() );
}


CSceneMain::~CSceneMain()
{
	delete mDoorsAnim;
	stl_utils::wipe( mRoom );
	stl_utils::wipe( mRoom2 );
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

	SVector3 dofPos;
	float	 dofScale;

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
		mDOFAnimPos->sample( mCurrAnimAlpha, 0, 1, &dofPos );
		mDOFAnimRange->sample( mCurrAnimAlpha, 0, 1, &dofScale );
	} else {
		SVector3 pos1, pos2;
		SQuaternion rot1, rot2;
		SVector3 params1, params2;
		SVector3 dpos1, dpos2;
		float range1, range2;
		double a1 = mCurrAnimAlpha - (3.0/mAnimFrameCount);
		double a2 = mCurrAnimAlpha - (2.5/mAnimFrameCount);
		double lerper = (mCurrAnimAlpha-a1) / (a2-a1);
		mCameraAnimPos->sample( a1, 0, 1, &pos1 );
		mCameraAnimPos->sample( a2, 0, 1, &pos2 );
		mCameraAnimRot->sample( a1, 0, 1, &rot1 );
		mCameraAnimRot->sample( a2, 0, 1, &rot2 );
		mCameraAnimParams->sample( a1, 0, 1, &params1 );
		mCameraAnimParams->sample( a2, 0, 1, &params2 );
		mDOFAnimPos->sample( a1, 0, 1, &dpos1 );
		mDOFAnimPos->sample( a2, 0, 1, &dpos2 );
		mDOFAnimRange->sample( a1, 0, 1, &range1 );
		mDOFAnimRange->sample( a2, 0, 1, &range2 );
		camPos = math_lerp<SVector3>( pos1, pos2, lerper );
		camRot = math_lerp<SQuaternion>( rot1, rot2, lerper );
		camParams = math_lerp<SVector3>( params1, params2, lerper );
		dofPos = math_lerp<SVector3>( dpos1, dpos2, lerper );
		dofScale = math_lerp<float>( range1, range2, lerper );
	}

	const float fov = camParams.z;

	SMatrix4x4 mr;
	D3DXMatrixRotationX( &mr, D3DX_PI/2 );
	getCamera().mWorldMat = mr * SMatrix4x4( camPos, camRot );

	const float camnear = 0.1f; // not from animation, just hardcoded
	const float camfar = 50.0f;

	float aspect = CD3DDevice::getInstance().getBackBufferAspect();
	getCamera().setProjectionParams( fov / aspect, aspect, camnear, camfar );

	// DOF
	const SVector3 toFocus = dofPos - getCamera().mWorldMat.getOrigin();
	const float dofDist = toFocus.dot( getCamera().mWorldMat.getAxisZ() );
	const float dofRange = dofScale * 1.2f;
	gDOFParams.set( dofDist, 1.0f / dofRange, 0.0f );
}


void CSceneMain::animateLocally( CComplexStuffEntity& e, float beginFrame )
{
	if( mCurrAnimFrame >= beginFrame ) {
		double animS = (mCurrAnimFrame-beginFrame)/ANIM_FPS;
		if( animS < 0 ) animS = 0;
		time_value animTime = time_value::fromsec( animS );
		e.update( animTime );
	}
}
void CSceneMain::animateLocally( CComplexStuffEntity& e, float beginFrame, float endFrame )
{
	if( mCurrAnimFrame >= beginFrame && mCurrAnimFrame <= endFrame ) {
		double animS = (mCurrAnimFrame-beginFrame)/ANIM_FPS;
		if( animS < 0 ) animS = 0;
		time_value animTime = time_value::fromsec( animS );
		e.update( animTime );
	}
}


void CSceneMain::update( time_value demoTime, float dt )
{
	mSharedStuff->updatePhysics();

	float demoTimeS = demoTime.tosec();
	mCurrAnimAlpha = demoTimeS / mAnimDuration;
	mCurrAnimFrame = mCurrAnimAlpha * mAnimFrameCount;

	bool secondLight = (mCurrAnimFrame >= LIGHT_SWITCH_FRAME);

	// animate characters
	gCharTimeBlend = clamp( (mCurrAnimFrame-TIMEBLEND_BEGIN_FRAME)/(TIMEBLEND_END_FRAME-TIMEBLEND_BEGIN_FRAME) );
	mCharacter->setLightPos( secondLight ? LIGHT_POS_2 : LIGHT_POS_1 );
	mCharacter->update( demoTime );

	animateLocally( *mCharacter2, GUY2_BEGIN_FRAME );
	animateLocally( *mCharacter3, GUY3_BEGIN_FRAME );

	// attacks
	animateLocally( *mAttack2_1, ATK2_BEGIN_FRAME, ATK2_END_FRAME );
	animateLocally( *mAttack2_2, ATK2_BEGIN_FRAME, ATK2_END_FRAME );

	// animate doors
	{
		// evaluate animation
		double animS = (mCurrAnimFrame-DOOR_BEGIN_FRAME)/ANIM_FPS;
		if( animS < 0 ) animS = 0;
		time_value animTime = time_value::fromsec( animS );
		mDoorsAnim->updateLocal( animTime );
		mDoorsAnim->updateWorld();
		// update corresponding room entities
		for( int i = 0; i < mDoorAnim2RoomIdx.size(); ++i ) {
			int idx = mDoorAnim2RoomIdx[i];
			if( idx < 0 )
				continue;
			mRoom[idx]->mWorldMat = mDoorsAnim->getBoneWorldMatrices()[i];
			mRoom[idx]->setMoved();
		}
	}
	

	animateLocally( *mBedAnim, BED_FRACTURE_FRAME );

	if( mCurrAnimFrame >= STONE_SHOW_FRAME && mCurrAnimFrame <= STONE_HIDE_FRAME ) {
		double stoneAnimS = (mCurrAnimFrame-STONE_BEGIN_FRAME)/ANIM_FPS;
		if( stoneAnimS < 0.0 )
			stoneAnimS = 0.0;
		time_value stoneAnimTime = time_value::fromsec( stoneAnimS );
		mStone->update( stoneAnimTime );
	}

	// update rooms
	int i;
	int n = mRoom.size();
	for( i = 0; i < n; ++i ) {
		mRoom[i]->update( LIGHT_POS_1 );
	}
	if( mCurrAnimFrame >= ROOM2_BEGIN_FRAME ) {
		n = mRoom2.size();
		for( i = 0; i < n; ++i ) {
			mRoom2[i]->update( LIGHT_POS_2 );
		}
	}

	int wallsLod = mCurrAnimFrame < WALL_LOD1_FRAME ? 0 : 1;

	gUpdateFractureScenario( mCurrAnimFrame, demoTimeS, wallsLod, mSharedStuff->getWalls(wallsLod) );

	mSharedStuff->updateFracture( wallsLod, demoTimeS );

	animateCamera();
}


void CSceneMain::render( eRenderMode renderMode )
{
	int wallsLod = mCurrAnimFrame < WALL_LOD1_FRAME ? 0 : 1;
	mSharedStuff->renderWalls( wallsLod, renderMode, mCurrAnimFrame >= FLOOR_FR_FRAME );

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
	if( mCurrAnimFrame >= ROOM2_BEGIN_FRAME ) {
		if( renderMode != RM_REFLECTED ) {
			n = mRoom2.size();
			for( i = 0; i < n; ++i ) {
				mRoom2[i]->render( renderMode );
			}
		}
		mRoom2Top->render( renderMode );
		mRoom2Bottom->render( renderMode );
	}

	// bed
	if( mCurrAnimFrame < BED_FRACTURE_FRAME )
		mBedStatic->render( renderMode );
	else if( mCurrAnimFrame < BED_HIDE_FRAME )
		mBedAnim->render( renderMode );

	// stone
	if( mCurrAnimFrame > STONE_SHOW_FRAME && mCurrAnimFrame < STONE_HIDE_FRAME )
		mStone->render( renderMode );

	// attacks
	if( mCurrAnimFrame > ATK2_BEGIN_FRAME && mCurrAnimFrame < ATK2_END_FRAME ) {
		mAttack2_1->render( renderMode );
		mAttack2_2->render( renderMode );
	}
}

const SMatrix4x4* CSceneMain::getLightTargetMatrix() const
{
	return &mCharacter->getAnimator().getBoneWorldMatrices()[mSpineBoneIndex];
}
