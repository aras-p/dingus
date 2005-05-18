#ifndef __SCENE_SHARED_H
#define __SCENE_SHARED_H

#include "DemoResources.h"
#include "Entity.h"


class CWall3D;
class CMeshEntity;
namespace dingus {
	struct SLine3;
};


class CSceneSharedStuff : public boost::noncopyable {
public:
	CSceneSharedStuff();
	~CSceneSharedStuff();

	void	renderWalls( int lodIndex, eRenderMode rm );
	void	updatePhysics();
	void	updateFracture( int lodIndex, float demoTimeS );

	bool	cullWall( int i, const SMatrix4x4& viewProj ) const;

	CCameraEntity& getWallCamera() { return mWallCamera; }
	CWall3D** getWalls( int lodIndex ) { return mWalls[lodIndex]; }

	float	intersectRay( const SLine3& ray ) const;
	void	fractureSphere( float demoTimeS, const SVector3& pos, float radius );

private:
	CCameraEntity	mWallCamera;
	CWall3D*		mWalls[2][CFACE_COUNT]; // some can be NULL
	
	CMeshEntity*	mFixWallPY;
	CMeshEntity*	mFixWallNY;
};



#endif
