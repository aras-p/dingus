#ifndef __SCENE_SHARED_H
#define __SCENE_SHARED_H

#include "DemoResources.h"
#include "Entity.h"


class CWall3D;



class CSceneSharedStuff : public boost::noncopyable {
public:
	CSceneSharedStuff();
	~CSceneSharedStuff();

	void	renderWalls( int lodIndex, eRenderMode rm );
	void	updatePhysics();
	void	updateFracture( int lodIndex, float demoTimeS );

	CCameraEntity& getWallCamera() { return mWallCamera; }
	CWall3D** getWalls( int lodIndex ) { return mWalls[lodIndex]; }

private:
	CCameraEntity	mWallCamera;
	CWall3D*		mWalls[2][CFACE_COUNT];
};



#endif
