#include "stdafx.h"
#include "SceneShared.h"
#include "wallz/WallPieces.h"
#include "wallz/WallPhysics.h"
#include "wallz/WallFracturer.h"
#include <dingus/utils/FixedRateProcess.h>



const float PHYS_UPDATE_FREQ = 60.0f;
const float PHYS_UPDATE_DT = 1.0f / PHYS_UPDATE_FREQ;


class CPhysicsProcess : public CFixedRateProcess {
public:
	CPhysicsProcess() : CFixedRateProcess( PHYS_UPDATE_FREQ, 50 ) { }
protected:
	virtual void performProcess() {
		wall_phys::update();
	};
};

CPhysicsProcess	gPhysProcess;



CSceneSharedStuff::CSceneSharedStuff()
{
	// walls
	const float ELEM_SIZE1 = 0.1f;
	const float ELEM_SIZE2 = 0.15f;

	mWalls[0][CFACE_PX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PX] );
	mWalls[0][CFACE_NX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NX] );
	mWalls[0][CFACE_PY] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.z), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PY] );
	mWalls[0][CFACE_NY] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.z), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NY] );
	mWalls[0][CFACE_PZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PZ] );
	mWalls[0][CFACE_NZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NZ] );
	mWalls[1][CFACE_PX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PX] );
	mWalls[1][CFACE_NX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NX] );
	mWalls[1][CFACE_PY] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.z), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PY] );
	mWalls[1][CFACE_NY] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.z), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NY] );
	mWalls[1][CFACE_PZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PZ] );
	mWalls[1][CFACE_NZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NZ] );

	SMatrix4x4 wm;
	wm.identify();
	
	wm.getAxisX().set( 0, 0, 1 );
	wm.getAxisY().set( 0, 1, 0 );
	wm.getAxisZ().set( -1, 0, 0 );
	wm.getOrigin().set( ROOM_MAX.x, ROOM_MIN.y, ROOM_MIN.z );
	mWalls[0][CFACE_PX]->setMatrix( wm );
	mWalls[1][CFACE_PX]->setMatrix( wm );
	wm.getAxisX().set( 0, 0, -1 );
	wm.getAxisY().set( 0, 1, 0 );
	wm.getAxisZ().set( 1, 0, 0 );
	wm.getOrigin().set( ROOM_MIN.x, ROOM_MIN.y, ROOM_MAX.z );
	mWalls[0][CFACE_NX]->setMatrix( wm );
	mWalls[1][CFACE_NX]->setMatrix( wm );
	wm.getAxisX().set( 1, 0, 0 );
	wm.getAxisY().set( 0, 0, 1 );
	wm.getAxisZ().set( 0, -1, 0 );
	wm.getOrigin().set( ROOM_MIN.x, ROOM_MAX.y, ROOM_MIN.z );
	mWalls[0][CFACE_PY]->setMatrix( wm );
	mWalls[1][CFACE_PY]->setMatrix( wm );
	wm.getAxisX().set( 1, 0, 0 );
	wm.getAxisY().set( 0, 0, -1 );
	wm.getAxisZ().set( 0, 1, 0 );
	wm.getOrigin().set( ROOM_MIN.x, ROOM_MIN.y, ROOM_MAX.z );
	mWalls[0][CFACE_NY]->setMatrix( wm );
	mWalls[1][CFACE_NY]->setMatrix( wm );
	wm.getAxisX().set( -1, 0, 0 );
	wm.getAxisY().set( 0, 1, 0 );
	wm.getAxisZ().set( 0, 0, -1 );
	wm.getOrigin().set( ROOM_MAX.x, ROOM_MIN.y, ROOM_MAX.z );
	mWalls[0][CFACE_PZ]->setMatrix( wm );
	mWalls[1][CFACE_PZ]->setMatrix( wm );
	wm.getAxisX().set( 1, 0, 0 );
	wm.getAxisY().set( 0, 1, 0 );
	wm.getAxisZ().set( 0, 0, 1 );
	wm.getOrigin().set( ROOM_MIN.x, ROOM_MIN.y, ROOM_MIN.z );
	mWalls[0][CFACE_NZ]->setMatrix( wm );
	mWalls[1][CFACE_NZ]->setMatrix( wm );

	int i;

	// first compute hi-detail walls
	for( i = 0; i < CFACE_COUNT; ++i ) {
		wallFractureCompute( mWalls[0][i]->getWall2D() );
	}
	// then low-detail ones (so that it doesn't mess up randoms in)
	for( i = 0; i < CFACE_COUNT; ++i ) {
		wallFractureCompute( mWalls[1][i]->getWall2D() );
	}

	wall_phys::initialize( PHYS_UPDATE_DT, ROOM_MIN-SVector3(1.0f,1.0f,1.0f), ROOM_MAX+SVector3(1.0f,1.0f,1.0f) );

	for( i = 0; i < CFACE_COUNT; ++i ) {
		wall_phys::addWall( 0, *mWalls[0][i] );
		wall_phys::addWall( 1, *mWalls[1][i] );
	}

	for( i = 0; i < CFACE_COUNT; ++i ) {
		mWalls[0][i]->update( 0.0f );
		mWalls[1][i]->update( 0.0f );
	}
}

CSceneSharedStuff::~CSceneSharedStuff()
{
	wall_phys::shutdown();
	for( int i = 0; i < CFACE_COUNT; ++i ) {
		delete mWalls[0][i];
		delete mWalls[1][i];
	}
}


void CSceneSharedStuff::renderWalls( int lodIndex, eRenderMode rm )
{
	for( int i = 0; i < CFACE_COUNT; ++i ) {
		mWalls[lodIndex][i]->render( rm );
	}
	wall_phys::render( rm );
}


void CSceneSharedStuff::updatePhysics()
{
	gPhysProcess.perform();
}

void CSceneSharedStuff::updateFracture( int lodIndex, float demoTimeS )
{
	for( int i = 0; i < CFACE_COUNT; ++i ) {
		mWalls[lodIndex][i]->update( demoTimeS );
	}
}

