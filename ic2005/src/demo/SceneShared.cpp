#include "stdafx.h"
#include "SceneShared.h"
#include "MeshEntity.h"
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



static const char* WALL_RESTEXS[CFACE_COUNT] = {
	"wres_px", "wres_nx", "wres_py", "wres_ny", "wres_pz", "wres_nz",
};



CSceneSharedStuff::CSceneSharedStuff()
{
	// walls
	const float ELEM_SIZE1 = 0.1f;
	const float ELEM_SIZE2 = 0.15f;

	CSharedTextureBundle& stb = CSharedTextureBundle::getInstance();
	ITextureCreator* tcr = new CFixedTextureCreator(CWall3D::RESGRID_X,CWall3D::RESGRID_Y,1,0,D3DFMT_A8,D3DPOOL_MANAGED);

	stb.registerTexture( WALL_RESTEXS[CFACE_PX], *tcr );
	stb.registerTexture( WALL_RESTEXS[CFACE_NX], *tcr );
	stb.registerTexture( WALL_RESTEXS[CFACE_PZ], *tcr );
	stb.registerTexture( WALL_RESTEXS[CFACE_NZ], *tcr );

	mWalls[0][CFACE_PX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PX], WALL_RESTEXS[CFACE_PX] );
	mWalls[0][CFACE_NX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NX], WALL_RESTEXS[CFACE_NX] );
	mWalls[0][CFACE_PY] = NULL;
	mWalls[0][CFACE_NY] = NULL;
	mWalls[0][CFACE_PZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PZ], WALL_RESTEXS[CFACE_PZ] );
	mWalls[0][CFACE_NZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE1, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NZ], WALL_RESTEXS[CFACE_NZ] );
	mWalls[1][CFACE_PX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PX], WALL_RESTEXS[CFACE_PX] );
	mWalls[1][CFACE_NX] = new CWall3D( SVector2(ROOM_SIZE.z,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NX], WALL_RESTEXS[CFACE_NX] );
	mWalls[1][CFACE_PY] = NULL;
	mWalls[1][CFACE_NY] = NULL;
	mWalls[1][CFACE_PZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_PZ], WALL_RESTEXS[CFACE_PZ] );
	mWalls[1][CFACE_NZ] = new CWall3D( SVector2(ROOM_SIZE.x,ROOM_SIZE.y), ELEM_SIZE2, gNoPixelShaders ? NULL : WALL_TEXS[CFACE_NZ], WALL_RESTEXS[CFACE_NZ] );

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

	SMatrix4x4 wallPYMat;
	wallPYMat.getAxisX().set( 1, 0, 0 );
	wallPYMat.getAxisY().set( 0, 0, 1 );
	wallPYMat.getAxisZ().set( 0, -1, 0 );
	wallPYMat.getOrigin().set( ROOM_MIN.x, ROOM_MAX.y, ROOM_MIN.z );
	SMatrix4x4 wallNYMat;
	wallNYMat.getAxisX().set( 1, 0, 0 );
	wallNYMat.getAxisY().set( 0, 0, -1 );
	wallNYMat.getAxisZ().set( 0, 1, 0 );
	wallNYMat.getOrigin().set( ROOM_MIN.x, ROOM_MIN.y, ROOM_MAX.z );
	/*
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
	*/

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
		if( mWalls[0][i] )
			wallFractureCompute( mWalls[0][i]->getWall2D() );
	}
	// then low-detail ones (so that it doesn't mess up randoms in)
	for( i = 0; i < CFACE_COUNT; ++i ) {
		if( mWalls[1][i] )
			wallFractureCompute( mWalls[1][i]->getWall2D() );
	}

	wall_phys::initialize( PHYS_UPDATE_DT, ROOM_MIN-SVector3(1.0f,1.0f,1.0f), ROOM_MAX+SVector3(1.0f,1.0f,1.0f) );

	for( i = 0; i < CFACE_COUNT; ++i ) {
		if( mWalls[0][i] )
			wall_phys::addWall( 0, *mWalls[0][i] );
		else
			wall_phys::addStaticWall( 0, i==CFACE_PY ? wallPYMat : wallNYMat );

		if( mWalls[1][i] )
			wall_phys::addWall( 1, *mWalls[1][i] );
		else
			wall_phys::addStaticWall( 1, i==CFACE_PY ? wallPYMat : wallNYMat );
	}

	for( i = 0; i < CFACE_COUNT; ++i ) {
		if( mWalls[0][i] )
			mWalls[0][i]->update( 0.0f );
		if( mWalls[1][i] )
			mWalls[1][i]->update( 0.0f );
	}

	// fixed walls
	mFixWallPY = new CMeshEntity( "RoomCeil" );
	mFixWallNY = new CMeshEntity( "RoomFloor" );
	if( !gNoPixelShaders ) {
		mFixWallPY->getRenderMesh(RM_NORMAL)->getParams().addTexture( "tRefl", *RGET_S_TEX(RT_REFL_PY) );
		mFixWallNY->getRenderMesh(RM_NORMAL)->getParams().addTexture( "tRefl", *RGET_S_TEX(RT_REFL_NY) );
	}
}

CSceneSharedStuff::~CSceneSharedStuff()
{
	delete mFixWallPY;
	delete mFixWallNY;

	wall_phys::shutdown();
	for( int i = 0; i < CFACE_COUNT; ++i ) {
		safeDelete( mWalls[0][i] );
		safeDelete( mWalls[1][i] );
	}
}


void CSceneSharedStuff::renderWalls( int lodIndex, eRenderMode rm )
{
	for( int i = 0; i < CFACE_COUNT; ++i ) {
		if( mWalls[lodIndex][i] ) {
			mWalls[lodIndex][i]->render( rm );
		} else {
			if( i == CFACE_NY )
				mFixWallNY->render( rm );
			else if( i == CFACE_PY )
				mFixWallPY->render( rm );
		}
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
		if( mWalls[lodIndex][i] )
			mWalls[lodIndex][i]->update( demoTimeS );
	}
}

